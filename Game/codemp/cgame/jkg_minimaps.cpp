/////////////////////////////////////////////
//
// Minimap handling code
// By BobaFett
//

#include "cg_local.h"
#include "../ui/ui_shared.h"
#include "json/cJSON.h"

typedef struct {
	vec3_t		mins;		// Mins of the area that should use this minimap
	vec3_t		maxs;		// Maxs of the area that should use this minimap
	int			priority;	// Priority of the minimap, higher = use in case of multiple matches
	qhandle_t	shader;		// Shader to use
	float		x;			// X game coord of the top-left corner
	float		y;			// Y game coord of the top-left corner
	float		w;			// Width of the minimap in game coords
	float		h;			// Height of the minimap in game coords
	float		radius;		// Default radius (in game units)
	qboolean	xinvert;	// Whether the x-axis is inverted (if the X coord from left to right go down)
	qboolean	yinvert;	// Whether the y-axis is inverted (if the Y coord from top to bottom go down)
}  mm_area_t;

// Local data
static struct {
	int			maploaded;
	int			areacount;
	mm_area_t	*areas;
	int			currentarea;
	int			newarea;
	int			fadetime;
	int			lastupdate;
	qhandle_t	maskshader;
	qhandle_t	nomapshader;
	qhandle_t	overlayshader;
} minimap_data;

extern displayContextDef_t cgDC;


static void MiniMap_ParseArea(cJSON *area) {
	mm_area_t *entry;
	cJSON *item;

	minimap_data.areacount++;
	minimap_data.areas = (mm_area_t *)realloc( minimap_data.areas, sizeof(mm_area_t) * minimap_data.areacount );
	
	entry = &minimap_data.areas[minimap_data.areacount-1];
	if (!minimap_data.areas) {
		// ..crap...outta memory? are you kidding me?
		CG_Printf("CRITITAL ERROR: Could not allocate memory for minimap area! (minimaps/%s.mmd)\n", cgs.rawmapname);
		return;
	}

	item = cJSON_GetObjectItem(area, "mins");
	if (!item || !cJSON_IsArray(item)) {
		CG_Printf("WARNING: Area without 'mins' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		cJSON_ReadFloatArray(item, 3, entry->mins);
	}
	
	item = cJSON_GetObjectItem(area, "maxs");
	if (!item || !cJSON_IsArray(item)) {
		CG_Printf("WARNING: Area without 'maxs' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		cJSON_ReadFloatArray(item, 3, entry->maxs);
	}

	item = cJSON_GetObjectItem(area, "priority");
	if (!item || !cJSON_IsNumber(item)) {
		CG_Printf("WARNING: Area without 'priority' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->priority = cJSON_ToInteger(item);
	}

	item = cJSON_GetObjectItem(area, "map");
	if (!item || !cJSON_IsString(item)) {
		CG_Printf("WARNING: Area without 'map' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->shader = trap_R_RegisterShader(cJSON_ToStringOpt(item, "*white"));
	}

	item = cJSON_GetObjectItem(area, "mapbase");
	if (!item || !cJSON_IsArray(item)) {
		CG_Printf("WARNING: Area without 'mapbase' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		cJSON_ReadFloatArray(item, 2, &entry->x);
	}

	item = cJSON_GetObjectItem(area, "width");
	if (!item || !cJSON_IsNumber(item)) {
		CG_Printf("WARNING: Area without 'width' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->w = cJSON_ToNumber(item);
	}

	item = cJSON_GetObjectItem(area, "height");
	if (!item || !cJSON_IsNumber(item)) {
		CG_Printf("WARNING: Area without 'height' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->h = cJSON_ToNumber(item);
	}

	item = cJSON_GetObjectItem(area, "xinvert");
	if (!item || !cJSON_IsBoolean(item)) {
		CG_Printf("WARNING: Area without 'xinvert' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->xinvert = cJSON_ToBoolean(item);
	}

	item = cJSON_GetObjectItem(area, "yinvert");
	if (!item || !cJSON_IsBoolean(item)) {
		CG_Printf("WARNING: Area without 'yinvert' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->yinvert = cJSON_ToBoolean(item);
	}
	
	item = cJSON_GetObjectItem(area, "radius");
	if (!item || !cJSON_IsNumber(item)) {
		CG_Printf("WARNING: Area without 'radius' in minimaps/%s.mmd\n", cgs.rawmapname);
	} else {
		entry->radius = cJSON_ToNumber(item);
	}
}

void MiniMap_Init() {
	fileHandle_t f;
	int len;
	char *buff;
	char error[256];
	cJSON *root;
	cJSON *areas;
	int i;

	// First, lets load some shaders :)
	memset(&minimap_data, 0, sizeof(minimap_data));

	minimap_data.maskshader = trap_R_RegisterShaderNoMip("minimaps/mask");
	minimap_data.overlayshader = trap_R_RegisterShaderNoMip("gfx/jkghud/minimap_visionmask.png");
	//minimap_data.nomapshader = trap_R_RegisterShader("gfx/jkghud/minimap_nomap.png");
	minimap_data.nomapshader = trap_R_RegisterShader("gfx/jkghud/minimap_backup_radar.png");

	len = trap_FS_FOpenFile(va("minimaps/%s.mmd", cgs.rawmapname), &f, FS_READ);
	if (len < 1) {
		return;		// No minimap
	}
	// Awesome, we got a minimap def file, lets parse it
	buff = (char *)malloc(len+1);
	trap_FS_Read(buff,len,f);
	buff[len] = 0;
	trap_FS_FCloseFile(f);

	root = cJSON_ParsePooled(buff, error, sizeof(error));
	if (!root) {
		CG_Printf("Could not parse %s.mmd: %s\n", cgs.rawmapname, error);
		return;
	}

	areas = cJSON_GetObjectItem(root, "areas");
	if (!areas || !cJSON_IsArray(areas)) {
		cJSON_Delete(root);
		CG_Printf("Could not parse %s.mmd: No 'areas' section not found\n", cgs.rawmapname);
		return;
	}

	for (i=0; i< cJSON_GetArraySize(areas); i++) {
		MiniMap_ParseArea(cJSON_GetArrayItem(areas, i));
	}

	minimap_data.maploaded = 1;
	minimap_data.currentarea = 1;
	
	cJSON_Delete(root);
	
	free(buff);
}

static qboolean MiniMap_PointInBox(vec3_t point, vec3_t mins, vec3_t maxs) {

	if(point[0]>maxs[0])
		return qfalse;

	if(point[1]>maxs[1])
		return qfalse;

	if(point[2]>maxs[2])
		return qfalse;

	if(point[0]<mins[0])
		return qfalse;

	if(point[1]<mins[1])
		return qfalse;

	if(point[2]<mins[2])
		return qfalse;

	return qtrue;
}

static void MiniMap_UpdateMapArea() {
	vec3_t porg;
	int lastPriority = -1;
	int newArea = -1;
	int i;
	mm_area_t *entry;

	VectorCopy(cg.predictedPlayerState.origin, porg);

	for (i=0; i<minimap_data.areacount; i++) {
		entry = &minimap_data.areas[i];
		if (!MiniMap_PointInBox(porg, entry->mins, entry->maxs))
			continue;
		if (entry->priority > lastPriority) {
			lastPriority = entry->priority;
			newArea = i;
		}
	}
	if (newArea != minimap_data.currentarea && !minimap_data.fadetime) {
		minimap_data.newarea = newArea;
		minimap_data.fadetime = cg.time;
	}
}

qboolean HaveMiniMap( void )
{
	if (!minimap_data.maploaded)
		return qfalse;

	return qtrue;
}

void MiniMap_Render(menuDef_t *menu, float radiusScale) {
	itemDef_t *item;
	mm_area_t *entry;
	float radius;
	vec4_t overlayColor = {1,1,1,0.7f};
	vec4_t transcolor = {1,1,1,1};
	vec4_t opacity;
	
	MAKERGBA(opacity, 1, 1, 1, 1*cg.jkg_HUDOpacity);

	item = Menu_FindItemByName(menu, "maparea");
	if (!item) {
		return;
	}
	if (!minimap_data.maploaded) {
		// Render the blank one if we got no map data
		trap_R_SetColor(opacity);
		trap_R_DrawStretchPic(
			item->window.rect.x, 
			item->window.rect.y, 
			item->window.rect.w, 
			item->window.rect.h,
			0, 0, 1, 1,
			minimap_data.nomapshader);
	} else {
		// Alright! We got map data!
		if (cg.time - minimap_data.lastupdate > 250) {
			// Recheck the map to use every 250 msecs instead of every frame
			MiniMap_UpdateMapArea();
			minimap_data.lastupdate = cg.time;
		}
		if (minimap_data.currentarea == -1) {
			// Not in a valid area, so no minimap
			trap_R_SetColor(opacity);
			trap_R_DrawStretchPic(
				item->window.rect.x, 
				item->window.rect.y, 
				item->window.rect.w, 
				item->window.rect.h,
				0, 0, 1, 1,
				minimap_data.nomapshader);
		} else {
			float s0, s1, t0, t1;
			vec2_t temp;
			entry = &minimap_data.areas[minimap_data.currentarea];
			// Time to render the minimap

			// Store our X and Y coordinate		
			temp[0] = cg.predictedPlayerState.origin[0];
			temp[1] = cg.predictedPlayerState.origin[1];

			// Work out our unnormalized texel coordinates for our position
			// S/T0 = Horizontal texel
			// S/T1 = Vertical texel

			if (entry->xinvert) {
				s0 = entry->x - temp[0];
			} else {
				s0 = temp[0] - entry->x;
			}
			if (entry->yinvert) {
				s1 = entry->y - temp[1];				
			} else {
				s1 = temp[1] - entry->y;
			}
			
			radius = radiusScale * entry->radius;

			// Create a box around our position so our position is centered
			t0 = s0 + radius;
			t1 = s1 + radius;
			s0 -= radius;
			s1 -= radius;

			// Normalize our texel coordinate

			s0 = s0 / entry->w;
			t0 = t0 / entry->w;

			s1 = s1 / entry->h;
			t1 = t1 / entry->h;

			// Time to render it
			// First, reset our color
			trap_R_SetColor(opacity);
			
			// Next draw the alpha mask, this one uses an alpha func
			// So the next layer (which uses depthfunc) will inherit this alpha
			trap_R_DrawRotatePic2(
				item->window.rect.x+(item->window.rect.w/2),	// Center X
				item->window.rect.y+(item->window.rect.h/2),	// Center Y
				item->window.rect.w,								// Radius W
				item->window.rect.h,								// Radius H
				0, 0, 1, 1,												// Texels (full shader)
				0,														// No angle for this one
				minimap_data.maskshader);								// Mask shader

			// Now we draw our actual minimap picture over it
			// Cuz of the alpha layer we rendered, this shader will 
			// be clipped off in areas the alpha layer was transparent
			// This way we obtain the round shape
			trap_R_DrawRotatePic2( 
				item->window.rect.x+(item->window.rect.w/2),	// Center X
				item->window.rect.y+(item->window.rect.h/2),	// Center Y
				item->window.rect.w,								// Radius W
				item->window.rect.h,								// Radius H
				s0, s1, t0, t1,
				cg.refdef.viewangles[YAW] - 90.0f,
				entry->shader);

			
			// Now we just put our overlay on top of it
			// But only if we're not fading with another map
			if (!minimap_data.fadetime) {
				overlayColor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(overlayColor);
				trap_R_DrawStretchPic(
					item->window.rect.x, 
					item->window.rect.y, 
					item->window.rect.w, 
					item->window.rect.h,
					0, 0, 1, 1,
					minimap_data.overlayshader);
			}
			trap_R_SetColor(opacity);
		}
		if (minimap_data.fadetime) {
			// We're transitioning
			if (cg.time - minimap_data.fadetime > 250) {
				// We're done fading
				minimap_data.fadetime = 0;
				minimap_data.currentarea = minimap_data.newarea;
				transcolor[3] = 1;
			} else {
				transcolor[3] = ((float)(cg.time - minimap_data.fadetime))/250.0f;
			}
			if (minimap_data.newarea == -1) {
				// Not in a valid area, so no minimap
				transcolor[3] *= cg.jkg_HUDOpacity;
				trap_R_SetColor(transcolor);
				trap_R_DrawStretchPic(
					item->window.rect.x, 
					item->window.rect.y, 
					item->window.rect.w, 
					item->window.rect.h,
					0, 0, 1, 1,
					minimap_data.nomapshader);
			} else {
				float s0, s1, t0, t1;
				vec2_t temp;
				entry = &minimap_data.areas[minimap_data.newarea];
				// Time to render the minimap

				// Store our X and Y coordinate		
				temp[0] = cg.predictedPlayerState.origin[0];
				temp[1] = cg.predictedPlayerState.origin[1];

				// Work out our unnormalized texel coordinates for our position
				// S/T0 = Horizontal texel
				// S/T1 = Vertical texel

				if (entry->xinvert) {
					s0 = entry->x - temp[0];
				} else {
					s0 = temp[0] - entry->x;
				}
				if (entry->yinvert) {
					s1 = entry->y - temp[1];				
				} else {
					s1 = temp[1] - entry->y;
				}

				radius = radiusScale * entry->radius;
				
				// Create a box around our position so our position is centered
				t0 = s0 + radius;
				t1 = s1 + radius;
				s0 -= radius;
				s1 -= radius;

				// Normalize our texel coordinate

				s0 = s0 / entry->w;
				t0 = t0 / entry->w;

				s1 = s1 / entry->h;
				t1 = t1 / entry->h;

				// Time to render it
				// First, reset our color
				trap_R_SetColor(transcolor);

				// Now we draw our actual minimap picture over it
				// Cuz of the alpha layer we rendered, this shader will 
				// be clipped off in areas the alpha layer was transparent
				// This way we obtain the round shape
				trap_R_DrawRotatePic2( 
					item->window.rect.x+(item->window.rect.w/2),	// Center X
					item->window.rect.y+(item->window.rect.h/2),	// Center Y
					item->window.rect.w,								// Radius W
					item->window.rect.h,								// Radius H
					s0, s1, t0, t1,
					cg.refdef.viewangles[YAW] - 90.0f,
					entry->shader);

				// And put in the overlay (the first pass will skip this if we're fading
				trap_R_SetColor(overlayColor);
				trap_R_DrawStretchPic(
					item->window.rect.x, 
					item->window.rect.y, 
					item->window.rect.w, 
					item->window.rect.h,
					0, 0, 1, 1,
					minimap_data.overlayshader);
			
			}
		}
		// Now we flush the 'buffer' to ensure nothin else will be affected by the alpha func
		trap_R_DrawRotatePic2(0, 0, 0, 0, 0, 0, 0, 0, 0, cgs.media.whiteShader);
		trap_R_SetColor(opacity);
	}	
}