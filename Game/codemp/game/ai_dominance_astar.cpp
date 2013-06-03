// =======================================================================================================================================
//
//
//
//                       UNIQUEONE'S ALL NEW ULTRA UBER AWESOME ULTIMATE SUPER MAGICAL ASTAR PATHFINDER
//
//                        (actually not as fast as the old one, but provides much, much better routes)
//
//                         --------------------------------------------------------------------------
//
//                                                      TODO LIST:
//									   Multithread this baby to another/other core(s).
//                                 Work out some way to optimize? - My head hurts already!
//                    Waypoint Super Highways? - Save the long waypoint lists for fast access by other AI?
//           Buy a new mouse! - This one's buttons unclick at the wrong moments all the time - it's driving me insane!
//
//
// =======================================================================================================================================

// Disable stupid warnings...
#pragma warning( disable : 4710 )

#include <vector>

#include "g_local.h"
#include "ai_main.h"
#include "q_shared.h"

// Disable stupid warnings...
#pragma warning( disable : 4710 )

#define MAX_NODELINKS       32

extern int FRAME_TIME;
extern int gWPNum;
extern wpobject_t *gWPArray[MAX_WPARRAY_SIZE];

extern float HeightDistance ( vec3_t v1, vec3_t v2 );
extern int DOM_FindIdealPathtoWP(bot_state_t *bs, int from, int to, int badwp2, int *pathlist);
//extern void QDECL G_Printf( const char *msg, ... );

qboolean PATHING_IGNORE_FRAME_TIME = qfalse;

extern int BOT_GetFCost ( gentity_t *bot, int to, int num, int parentNum, float *gcost );


//
// UQ1: Oh man....
//
// I spent way too much time on this new pathing method... only to find the bug in my old one hahhahaha!!!
//
// ... anyway, we now have almost perfect routes in a fraction on the time of the "new" method 
// (*cough* 1 sec for all entitiy paths on map, compared to 45 secs with the "new" method for very little distance advantage)
//
// Anyway, I call this a major win!
//

//#define __SLOW_PATHING__

#ifdef __SLOW_PATHING__
struct SASNode
{
	bool	bIsOpen;
	bool	bIsClosed;
	int		ParentIndex;
	int		H;
	int		G;
	int		I;
	int		F;//G+H+I
	int		NeighborCount;
	int		Neighbor[MAX_NODELINKS];
	int		NeighborDistance[MAX_NODELINKS];
};

SASNode *pASNode;

void ASTAR_InitNodeByWayPoint()
{
	if (pASNode)
	{
		delete[] pASNode;
		pASNode = NULL;
	}

	int NodeCount = gWPNum;
	pASNode = new SASNode[NodeCount];

	for(int i = 0 ; i < NodeCount; i++)
	{
		pASNode[i].bIsClosed=false;
		pASNode[i].bIsOpen=false;
		pASNode[i].F=0;
		pASNode[i].G=0;
		pASNode[i].H=0;
		pASNode[i].I=0;
		pASNode[i].ParentIndex=-1;
		pASNode[i].NeighborCount=0;

		for(int j = 0 ; j < gWPArray[i]->neighbornum; j++)
		{
			float cost = Distance(gWPArray[i]->origin, gWPArray[gWPArray[i]->neighbors[j].num]->origin);

			// UQ1: Make Height Diffs matter (prefer single plane travel over the bumpy road)!!!
			float height_diff = HeightDistance(gWPArray[i]->origin, gWPArray[gWPArray[i]->neighbors[j].num]->origin);
			cost += (height_diff * height_diff); // Squared for massive preferance to staying at same plane...

			pASNode[i].NeighborCount = gWPArray[i]->neighbornum;
			pASNode[i].Neighbor[j] = gWPArray[i]->neighbors[j].num;
			pASNode[i].NeighborDistance[j] = cost;
		}
	}
}

bool SAS_INITIALIZED = false;

void ASTAR_InitSASNodes()
{
	if (!SAS_INITIALIZED)
	{
		ASTAR_InitNodeByWayPoint();
		SAS_INITIALIZED = true;
	}

	for (int i = 0; i < gWPNum; i++)
	{
		pASNode[i].bIsClosed=false;
		pASNode[i].bIsOpen=false;
		pASNode[i].F=0;
		pASNode[i].G=0;
		pASNode[i].H=0;
		pASNode[i].I=0;
		pASNode[i].ParentIndex=-1;
	}
}
	
int ASTAR_FindPath(int from, int to, int *pathlist)
{
	int PathPointCount = 0;
	std::vector<int>OpenList;
	std::vector<int>PathList;

	//
	// This version is completely unrestricted. ONLY use it if a path is EXTREMELY important! Pussible huge CPU spikes...
	//

	// Initialize...
	ASTAR_InitSASNodes();

	OpenList.clear();
	PathList.clear();
	int i=0;

	pASNode[from].bIsOpen=true;
	pASNode[from].G=0;
	OpenList.push_back(from);
		
	for(i = 0; i < gWPNum; i++)
	{
		pASNode[i].H=(int)sqrtl(_Pow_int(gWPArray[i]->origin[0] - gWPArray[to]->origin[0], 2) 
			+ _Pow_int(gWPArray[i]->origin[1] - gWPArray[to]->origin[1], 2)
			+ _Pow_int(gWPArray[i]->origin[2] - gWPArray[to]->origin[2], 2));
	}

	while(1)
	{
		bool bFoundPath=false;
		int NotClosedCount=0;

		for(i = 0; i < OpenList.size(); i++)
		{
			if(!pASNode[OpenList[i]].bIsClosed)
			{
				if(OpenList[i]==to)
				{
					bFoundPath=true;
					break;
				}

				NotClosedCount++;
			}
		}

		if(bFoundPath)
			break;

		if(NotClosedCount==0)
		{
			//G_Printf("JKG A*: Failed to find a normal path - trying backup A*\n");
			return -1; // Failed...
			//return DOM_FindIdealPathtoWP(NULL, from, to, -1, pathlist);
		}

		int MinFIndex=-1;
		int MinF=99999999;

		for(i=0;i<OpenList.size();i++)
		{
			int tempI=OpenList[i];
			if(!pASNode[tempI].bIsClosed)
			{
				int tempF=pASNode[tempI].G+pASNode[to].H;
				if(tempF<MinF)
				{
					MinF=tempF;
					MinFIndex=tempI;
				}
			}
		}

		for(int j=0;j<pASNode[MinFIndex].NeighborCount;j++)
		{
			SASNode *pTempChild=&pASNode[pASNode[MinFIndex].Neighbor[j]];

			if(pTempChild->bIsOpen)
			{
				if(pTempChild->G>pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j])
				{
					pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
					pTempChild->ParentIndex=MinFIndex;
				}
			}
			else
			{
				pTempChild->bIsOpen=true;
				OpenList.push_back(pASNode[MinFIndex].Neighbor[j]);
				pTempChild->ParentIndex=MinFIndex;
				pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
			}
		}

		pASNode[MinFIndex].bIsClosed=true;

		if(bFoundPath)
			break;
	}

	int tempIndex = to;
	PathList.push_back(tempIndex);
		
	while(tempIndex != from)
	{
		pathlist[PathPointCount] = tempIndex;
		PathPointCount++;

		tempIndex=pASNode[tempIndex].ParentIndex;
		PathList.push_back(tempIndex);
	}

	pathlist[PathPointCount] = from;
	PathPointCount++;

	/*
	G_Printf("=====================================================================");
	G_Printf("Path (length %i) found was:\n", PathPointCount);

	for (i = 0; i < PathPointCount; i++)
	{
		if (i == PathPointCount - 1)
			G_Printf("%i (goal) ", pathlist[i]);
		else
			G_Printf("%i (dist to next %f) ", pathlist[i], Distance(gWPArray[pathlist[i]]->origin, gWPArray[pathlist[i+1]]->origin));
	}

	G_Printf("\n");
	G_Printf("=====================================================================");
	*/

	//PathPointCount=PathList.size();
	return PathPointCount;
}

int ASTAR_FindPathWithTimeLimit(int from, int to, int *pathlist)
{
	int PathPointCount = 0;
	std::vector<int>OpenList;
	std::vector<int>PathList;
	int			startTime = trap_Milliseconds();

	// UQ1: Because this A* is only used to hunt enemies, limit the over-all range on requests to save CPU time...
	if (Distance(gWPArray[from]->origin, gWPArray[to]->origin) > 4096.0f) return -1;

	if (!PATHING_IGNORE_FRAME_TIME && trap_Milliseconds() - FRAME_TIME > 300)
	{// Never path on an already long frame time...
		return -1;
	}

	// Initialize...
	ASTAR_InitSASNodes();

	OpenList.clear();
	PathList.clear();
	int i=0;

	pASNode[from].bIsOpen=true;
	pASNode[from].G=0;
	OpenList.push_back(from);
		
	for(i = 0; i < gWPNum; i++)
	{
		pASNode[i].H=(int)sqrtl(_Pow_int(gWPArray[i]->origin[0] - gWPArray[to]->origin[0], 2) 
			+ _Pow_int(gWPArray[i]->origin[1] - gWPArray[to]->origin[1], 2)
			+ _Pow_int(gWPArray[i]->origin[2] - gWPArray[to]->origin[2], 2));
	}

	while(1)
	{
		bool bFoundPath=false;
		bool HaveNotClosed = false;

		for(i = 0; i < OpenList.size(); i++)
		{
			if(!pASNode[OpenList[i]].bIsClosed)
			{
				if(OpenList[i]==to)
				{
					bFoundPath=true;
					break;
				}

				HaveNotClosed = true;
				break;
			}
		}

		if(bFoundPath)
			break;

		if(!HaveNotClosed || trap_Milliseconds() - startTime > 500) // UQ1: Try limiting by timer, if we dont have one by now, we shouldn't bother!
		{
			//G_Printf("JKG A*: Failed to find a normal path - trying backup A*\n");
			return -1; // Failed...
			//return DOM_FindIdealPathtoWP(NULL, from, to, -1, pathlist);
		}

		int MinFIndex=-1;
		int MinF=99999999;

		for(i=0;i<OpenList.size();i++)
		{
			if(!pASNode[OpenList[i]].bIsClosed)
			{
				if(OpenList[i]==to)
				{
					bFoundPath=true;
					break;
				}
			}

			int tempI=OpenList[i];
			if(!pASNode[tempI].bIsClosed)
			{
				int tempF=pASNode[tempI].G+pASNode[to].H;
				if(tempF<MinF)
				{
					MinF=tempF;
					MinFIndex=tempI;
				}
			}
		}

		if(bFoundPath)
			break;

		for(int j=0;j<pASNode[MinFIndex].NeighborCount;j++)
		{
			SASNode *pTempChild=&pASNode[pASNode[MinFIndex].Neighbor[j]];

			if(pTempChild->bIsOpen)
			{
				if(pTempChild->G>pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j])
				{
					pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
					pTempChild->ParentIndex=MinFIndex;
				}
			}
			else
			{
				pTempChild->bIsOpen=true;
				OpenList.push_back(pASNode[MinFIndex].Neighbor[j]);
				pTempChild->ParentIndex=MinFIndex;
				pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
			}
		}

		pASNode[MinFIndex].bIsClosed=true;
	}

	int tempIndex = to;
	PathList.push_back(tempIndex);
		
	while(tempIndex != from)
	{
		pathlist[PathPointCount] = tempIndex;
		PathPointCount++;

		tempIndex=pASNode[tempIndex].ParentIndex;
		PathList.push_back(tempIndex);
	}

	pathlist[PathPointCount] = from;
	PathPointCount++;

	/*
	G_Printf("=====================================================================");
	G_Printf("Path (length %i) found was:\n", PathPointCount);

	for (i = 0; i < PathPointCount; i++)
	{
		if (i == PathPointCount - 1)
			G_Printf("%i (goal) ", pathlist[i]);
		else
			G_Printf("%i (dist to next %f) ", pathlist[i], Distance(gWPArray[pathlist[i]]->origin, gWPArray[pathlist[i+1]]->origin));
	}

	G_Printf("\n");
	G_Printf("=====================================================================");
	*/

	//PathPointCount=PathList.size();
	return PathPointCount;
}

//
//
//
//
//
//
//
//
//
//

void ASTAR_InitSASNodesShorten(int old_pathlist_size, int *old_pathlist)
{
	if (!SAS_INITIALIZED)
	{
		ASTAR_InitNodeByWayPoint();
		SAS_INITIALIZED = true;
	}

	for (int i = 0; i < old_pathlist_size; i++)
	{
		pASNode[old_pathlist[i]].bIsClosed=false;
		pASNode[old_pathlist[i]].bIsOpen=false;
		pASNode[old_pathlist[i]].F=0;
		pASNode[old_pathlist[i]].G=0;
		pASNode[old_pathlist[i]].H=0;
		pASNode[old_pathlist[i]].I=0;
		pASNode[old_pathlist[i]].ParentIndex=-1;
	}
}

int ORIGINAL_SIZE = 0;

int ASTAR_ShortenPath(int old_pathlist_size, int *old_pathlist, int *pathlist)
{
	int PathPointCount = 0;
	std::vector<int>OpenList;
	std::vector<int>PathList;
	int startTime = trap_Milliseconds();

	ORIGINAL_SIZE = old_pathlist_size;

	// Initialize...
	ASTAR_InitSASNodesShorten(old_pathlist_size, old_pathlist);

	//G_Printf("Path is %i nodes from node %i to node %i.\n", old_pathlist_size, old_pathlist[old_pathlist_size-1], old_pathlist[0]);

	OpenList.clear();
	PathList.clear();
	int i=0;

	pASNode[old_pathlist[old_pathlist_size-1]].bIsOpen=true;
	pASNode[old_pathlist[old_pathlist_size-1]].G=0;
	OpenList.push_back(old_pathlist[old_pathlist_size-1]);
		
	for(i = 0; i < old_pathlist_size; i++)
	{
		pASNode[old_pathlist[i]].H=(int)sqrtl(_Pow_int(gWPArray[i]->origin[0] - gWPArray[old_pathlist[0]]->origin[0], 2) 
			+ _Pow_int(gWPArray[i]->origin[1] - gWPArray[old_pathlist[0]]->origin[1], 2)
			+ _Pow_int(gWPArray[i]->origin[2] - gWPArray[old_pathlist[0]]->origin[2], 2));
	}

	while(1)
	{
		bool bFoundPath=false;
		int NotClosedCount=0;

		for(i = 0; i < OpenList.size(); i++)
		{
			if(!pASNode[OpenList[i]].bIsClosed)
			{
				if(OpenList[i]==old_pathlist[0])
				{
					bFoundPath=true;
					break;
				}

				NotClosedCount++;
			}
		}

		if(bFoundPath)
			break;

		if(NotClosedCount==0 || trap_Milliseconds()-startTime > 1000)
		{
			//G_Printf("JKG A*: Failed to find a normal path - trying backup A*\n");
			return -1; // Failed...
			//return DOM_FindIdealPathtoWP(NULL, from, to, -1, pathlist);
		}

		int MinFIndex=-1;
		int MinF=99999999;

		for(i=0;i<OpenList.size();i++)
		{
			int tempI=OpenList[i];
			if(!pASNode[tempI].bIsClosed)
			{
				int tempF=pASNode[tempI].G+pASNode[old_pathlist[0]].H;
				if(tempF<MinF)
				{
					MinF=tempF;
					MinFIndex=tempI;
				}
			}
		}

		for(int j=0;j<pASNode[MinFIndex].NeighborCount;j++)
		{
			SASNode *pTempChild=&pASNode[pASNode[MinFIndex].Neighbor[j]];

			if(pTempChild->bIsOpen)
			{
				if(pTempChild->G>pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j])
				{
					pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
					pTempChild->ParentIndex=MinFIndex;
				}
			}
			else
			{
				pTempChild->bIsOpen=true;
				OpenList.push_back(pASNode[MinFIndex].Neighbor[j]);
				pTempChild->ParentIndex=MinFIndex;
				pTempChild->G=pASNode[MinFIndex].G+pASNode[MinFIndex].NeighborDistance[j];
			}
		}

		pASNode[MinFIndex].bIsClosed=true;

		if(bFoundPath)
			break;
	}

	int tempIndex = old_pathlist[0];
	PathList.push_back(tempIndex);
		
	while(tempIndex != old_pathlist[old_pathlist_size-1])
	{
		pathlist[PathPointCount] = tempIndex;
		PathPointCount++;

		tempIndex=pASNode[tempIndex].ParentIndex;
		PathList.push_back(tempIndex);
	}

	pathlist[PathPointCount] = old_pathlist[old_pathlist_size-1];
	PathPointCount++;

	/*
	G_Printf("=====================================================================");
	G_Printf("Path (length %i) found was:\n", PathPointCount);

	for (i = 0; i < PathPointCount; i++)
	{
		if (i == PathPointCount - 1)
			G_Printf("%i (goal) ", pathlist[i]);
		else
			G_Printf("%i (dist to next %f) ", pathlist[i], Distance(gWPArray[pathlist[i]]->origin, gWPArray[pathlist[i+1]]->origin));
	}

	G_Printf("\n");
	G_Printf("=====================================================================");
	*/

	//PathPointCount=PathList.size();
	return PathPointCount;
}

#endif //__SLOW_PATHING__

#define NODE_INVALID -1
#define MAX_NODELINKS 32

extern int			*openlist;					//add 1 because it's a binary heap, and they don't use 0 - 1 is the first used index
extern float		*gcost;
extern int			*fcost;
extern char			*list;						//0 is neither, 1 is open, 2 is closed - char because it's the smallest data type
extern int			*parent;

extern void AllocatePathFindingMemory();

int ASTAR_FindPathFast(int from, int to, int *pathlist, qboolean shorten)
{
	//all the data we have to hold...since we can't do dynamic allocation, has to be MAX_WPARRAY_SIZE
	//we can probably lower this later - eg, the open list should never have more than at most a few dozen items on it
	int			badwp = -1;
	int			numOpen = 0;
	int			atNode, temp, newnode = -1;
	qboolean	found = qfalse;
	int			count = -1;
	float		gc;
	int			i, j, u, v, m;
	gentity_t	*bot = NULL;

	if (!PATHING_IGNORE_FRAME_TIME && trap_Milliseconds() - FRAME_TIME > 300)
	{// Never path on an already long frame time...
		return -1;
	}

	if ( (from == NODE_INVALID) || (to == NODE_INVALID) || (from >= gWPNum) || (to >= gWPNum) || (from == to) )
	{
		//G_Printf("Bad from or to node.\n");
		return ( -1 );
	}

	// Check if memory needs to be allocated...
	AllocatePathFindingMemory();

	memset( openlist, 0, (sizeof(int) * (gWPNum + 1)) );
	memset( gcost, 0, (sizeof(float) * gWPNum) );
	memset( fcost, 0, (sizeof(int) * gWPNum) );
	memset( list, 0, (sizeof(char) * gWPNum) );
	memset( parent, 0, (sizeof(int) * gWPNum) );

	for (i = 0; i < gWPNum; i++)
	{
		gcost[i] = Distance(gWPArray[i]->origin, gWPArray[to]->origin); 
	}

	openlist[gWPNum+1] = 0;

	openlist[1] = from;																	//add the starting node to the open list
	numOpen++;
	gcost[from] = 0;																	//its f and g costs are obviously 0
	fcost[from] = 0;

	while ( 1 )
	{
		if ( numOpen != 0 )																//if there are still items in the open list
		{
			//pop the top item off of the list
			atNode = openlist[1];
			list[atNode] = 2;															//put the node on the closed list so we don't check it again
			numOpen--;
			openlist[1] = openlist[numOpen + 1];										//move the last item in the list to the top position
			v = 1;

			//this while loop reorders the list so that the new lowest fcost is at the top again
			while ( 1 )
			{
				u = v;
				if ( (2 * u + 1) < numOpen )											//if both children exist
				{
					if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
					{
						v = 2 * u;
					}

					if ( fcost[openlist[v]] >= fcost[openlist[2 * u + 1]] )
					{
						v = 2 * u + 1;
					}
				}
				else
				{
					if ( (2 * u) < numOpen )											//if only one child exists
					{
						if ( fcost[openlist[u]] >= fcost[openlist[2 * u]] )
						{
							v = 2 * u;
						}
					}
				}

				if ( u != v )															//if they're out of order, swap this item with its parent
				{
					temp = openlist[u];
					openlist[u] = openlist[v];
					openlist[v] = temp;
				}
				else
				{
					break;
				}
			}

			for ( i = 0; i < gWPArray[atNode]->neighbornum && i < MAX_NODELINKS; i++ )								//loop through all the links for this node
			{
				newnode = gWPArray[atNode]->neighbors[i].num;

				if (newnode > gWPNum)
					continue;

				if (newnode < 0)
					continue;

				//if (nodes[newnode].objectNum[0] == 1)
				//	continue; // Skip water/ice disabled node!

				if ( list[newnode] == 2 )
				{																		//if this node is on the closed list, skip it
					continue;
				}

				if ( list[newnode] != 1 )												//if this node is not already on the open list
				{
					openlist[++numOpen] = newnode;										//add the new node to the open list
					list[newnode] = 1;
					parent[newnode] = atNode;											//record the node's parent

					if ( newnode == to )
					{																	//if we've found the goal, don't keep computing paths!
						break;															//this will break the 'for' and go all the way to 'if (list[to] == 1)'
					}

					fcost[newnode] = BOT_GetFCost( bot, to, newnode, parent[newnode], gcost );	//store it's f cost value

					//this loop re-orders the heap so that the lowest fcost is at the top
					m = numOpen;

					while ( m != 1 )													//while this item isn't at the top of the heap already
					{
						if ( fcost[openlist[m]] <= fcost[openlist[m / 2]] )				//if it has a lower fcost than its parent
						{
							temp = openlist[m / 2];
							openlist[m / 2] = openlist[m];
							openlist[m] = temp;											//swap them
							m /= 2;
						}
						else
						{
							break;
						}
					}
				}
				else										//if this node is already on the open list
				{
					gc = gcost[atNode];

					if (gWPArray[atNode]->neighbors[i].cost > 0 && gWPArray[atNode]->neighbors[i].cost < 9999)
					{// UQ1: Already have a cost value, skip the calculations!
						gc += gWPArray[atNode]->neighbors[i].cost;
					}
					else
					{
						vec3_t	vec;

						VectorSubtract( gWPArray[newnode]->origin, gWPArray[atNode]->origin, vec );
						gc += VectorLength( vec );				//calculate what the gcost would be if we reached this node along the current path
						gWPArray[atNode]->neighbors[i].cost = VectorLength( vec );

						/*
						float height_diff = 0.0f;
						float cost = 0.0f;

						cost = Distance(gWPArray[newnode]->origin, gWPArray[atNode]->origin);

						height_diff = HeightDistance(gWPArray[newnode]->origin, gWPArray[atNode]->origin);
						cost += (height_diff * height_diff); // Squared for massive preferance to staying at same plane...

						gWPArray[atNode]->neighbors[i].cost = cost;

						gc += cost;
						*/
					}

					if ( gc < gcost[newnode] )				//if the new gcost is less (ie, this path is shorter than what we had before)
					{
						parent[newnode] = atNode;			//set the new parent for this node
						gcost[newnode] = gc;				//and the new g cost

						for ( j = 1; j < numOpen; j++ )		//loop through all the items on the open list
						{
							if ( openlist[j] == newnode )	//find this node in the list
							{
								//calculate the new fcost and store it
								fcost[newnode] = BOT_GetFCost( bot, to, newnode, parent[newnode], gcost );

								//reorder the list again, with the lowest fcost item on top
								m = j;

								while ( m != 1 )
								{
									if ( fcost[openlist[m]] < fcost[openlist[m / 2]] )	//if the item has a lower fcost than it's parent
									{
										temp = openlist[m / 2];
										openlist[m / 2] = openlist[m];
										openlist[m] = temp;								//swap them
										m /= 2;
									}
									else
									{
										break;
									}
								}
								break;													//exit the 'for' loop because we already changed this node
							}															//if
						}																//for
					}											//if (gc < gcost[newnode])
				}												//if (list[newnode] != 1) --> else
			}													//for (loop through links)
		}														//if (numOpen != 0)
		else
		{
			found = qfalse;										//there is no path between these nodes
			break;
		}

		if ( list[to] == 1 )									//if the destination node is on the open list, we're done
		{
			found = qtrue;
			break;
		}
	}															//while (1)

	if ( found == qtrue )							//if we found a path, and are trying to store the pathlist...
	{
		count = 0;
		temp = to;												//start at the end point

		while ( temp != from )									//travel along the path (backwards) until we reach the starting point
		{
			if (count+1 >= MAX_WPARRAY_SIZE)
			{
				Com_Printf("ERROR: pathlist count > MAX_WPARRAY_SIZE.\n");
				return -1; // UQ1: Added to stop crash if path is too long for the memory allocation...
			}

			pathlist[count++] = temp;							//add the node to the pathlist and increment the count
			temp = parent[temp];								//move to the parent of this node to continue the path
		}

		pathlist[count++] = from;								//add the beginning node to the end of the pathlist

#ifdef __SLOW_PATHING__
		if (shorten)
		{// UQ1: Now use the path shortener on these waypoints...
			int pathlist_copy[MAX_WPARRAY_SIZE];
			memcpy(pathlist_copy, pathlist, sizeof(int)*(MAX_WPARRAY_SIZE));
			count = ASTAR_ShortenPath(count, pathlist_copy, pathlist);
		}
#endif //__SLOW_PATHING__

		//G_Printf("Pathsize is %i.\n", count);
		return ( count );
	}

	//G_Printf("Failed to find path.\n");
	return ( -1 );											//return the number of nodes in the path, -1 if not found
}

#ifndef __SLOW_PATHING__
int ASTAR_FindPath(int from, int to, int *pathlist)
{
	return ASTAR_FindPathFast(from, to, pathlist, qfalse);
}

int ASTAR_FindPathWithTimeLimit(int from, int to, int *pathlist)
{
	return ASTAR_FindPathFast(from, to, pathlist, qfalse);
}
#endif //__SLOW_PATHING__

extern int BG_GetTime();
extern int DOM_GetBestWaypoint(vec3_t org, int ignore, int badwp);

void AIMod_TimeMapPaths()
{
	int			startTime = trap_Milliseconds();
	/*short*/ int	pathlist[MAX_WPARRAY_SIZE];
	int			pathsize;
	gentity_t	*ent = NULL;
	int			i;
	int			current_wp, longTermGoal;
	int			NUM_PATHS = 0;
	int			PATH_DISTANCES[MAX_GENTITIES];
	int			TOTAL_DISTANCE = 0;
	int			AVERAGE_DISTANCE = 0;

	ent = G_Find(ent, FOFS(classname), "info_player_deathmatch");

	if (!ent)
		Com_Printf("No spawnpoint found!\n");

	current_wp = DOM_GetBestWaypoint(ent->r.currentOrigin, -1, -1);

	if (!current_wp)
		Com_Printf("No waypoint found!\n");

	Com_Printf( "Finding bot objectives at node number %i (%f %f %f).\n",
		current_wp, gWPArray[current_wp]->origin[0], gWPArray[current_wp]->origin[1],
		gWPArray[current_wp]->origin[2] );

	PATHING_IGNORE_FRAME_TIME = qtrue;

	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		gentity_t	*goal = &g_entities[i];

		if (!goal || !goal->inuse) continue;

		if (!goal->classname 
			|| !goal->classname[0] 
			|| !stricmp(goal->classname, "freed")
			|| !stricmp(goal->classname, "noclass")) 
			continue;

		if (i == ent->s.number) continue;

#ifdef __SLOW_PATHING__
		ORIGINAL_SIZE = 0;
#endif //__SLOW_PATHING__

		longTermGoal = DOM_GetBestWaypoint(goal->s.origin, -1, -1);

		//pathsize = ASTAR_FindPath(current_wp, longTermGoal, pathlist);
		//pathsize = ASTAR_FindPathWithTimeLimit(current_wp, longTermGoal, pathlist);
		//pathsize = ASTAR_FindPathFast(current_wp, longTermGoal, pathlist, qtrue);
		pathsize = ASTAR_FindPathFast(current_wp, longTermGoal, pathlist, qfalse);
		//pathsize = DOM_FindIdealPathtoWP(NULL, current_wp, longTermGoal, -1, pathlist);

		if (pathsize > 0)
		{
			PATH_DISTANCES[NUM_PATHS] = 0;

			for (int j = 0; j < pathsize-1; j++)
			{
				PATH_DISTANCES[NUM_PATHS] += Distance(gWPArray[pathlist[j]]->origin, gWPArray[pathlist[j+1]]->origin);
			}

			NUM_PATHS++;

#ifdef __SLOW_PATHING__
			if (ORIGINAL_SIZE > 0)
				Com_Printf( "Objective %i (%s) pathsize is %i (unshortened %i).\n", i, goal->classname, pathsize, ORIGINAL_SIZE );
			else
#endif //__SLOW_PATHING__
				Com_Printf( "Objective %i (%s) pathsize is %i.\n", i, goal->classname, pathsize );
		}
	}

	for (int j = 0; j < NUM_PATHS; j++)
	{
		TOTAL_DISTANCE += PATH_DISTANCES[j];
	}

	AVERAGE_DISTANCE = TOTAL_DISTANCE/NUM_PATHS;

	Com_Printf( "Completed %i paths in %i seconds. Average path distance is %i\n", NUM_PATHS, (int)((int)(trap_Milliseconds()-startTime)/1000), AVERAGE_DISTANCE );

#ifdef __SLOW_PATHING__

	//
	// And the alternative pathing...
	//

	startTime = trap_Milliseconds();
	NUM_PATHS = 0;
	PATH_DISTANCES[MAX_GENTITIES];
	TOTAL_DISTANCE = 0;
	AVERAGE_DISTANCE = 0;

	for ( i = 0; i < MAX_GENTITIES; i++ )
	{
		gentity_t	*goal = &g_entities[i];

		if (!goal || !goal->inuse) continue;

		if (!goal->classname 
			|| !goal->classname[0] 
			|| !stricmp(goal->classname, "freed")
			|| !stricmp(goal->classname, "noclass")) 
			continue;

		if (i == ent->s.number) continue;

		ORIGINAL_SIZE = 0;

		longTermGoal = DOM_GetBestWaypoint(goal->s.origin, -1, -1);

		pathsize = ASTAR_FindPath(current_wp, longTermGoal, pathlist);
		//pathsize = ASTAR_FindPathWithTimeLimit(current_wp, longTermGoal, pathlist);
		//pathsize = ASTAR_FindPathFast(current_wp, longTermGoal, pathlist, qtrue);
		//pathsize = ASTAR_FindPathFast(current_wp, longTermGoal, pathlist, qfalse);
		//pathsize = DOM_FindIdealPathtoWP(NULL, current_wp, longTermGoal, -1, pathlist);

		if (pathsize > 0)
		{
			PATH_DISTANCES[NUM_PATHS] = 0;

			for (int j = 0; j < pathsize-1; j++)
			{
				PATH_DISTANCES[NUM_PATHS] += Distance(gWPArray[pathlist[j]]->origin, gWPArray[pathlist[j+1]]->origin);
			}

			NUM_PATHS++;

			if (ORIGINAL_SIZE > 0)
				Com_Printf( "Objective %i (%s) pathsize is %i (unshortened %i).\n", i, goal->classname, pathsize, ORIGINAL_SIZE );
			else
				Com_Printf( "Objective %i (%s) pathsize is %i.\n", i, goal->classname, pathsize );
		}
	}

	for (int j = 0; j < NUM_PATHS; j++)
	{
		TOTAL_DISTANCE += PATH_DISTANCES[j];
	}

	AVERAGE_DISTANCE = TOTAL_DISTANCE/NUM_PATHS;

	Com_Printf( "Completed %i paths in %i seconds. Average path distance is %i\n", NUM_PATHS, (int)((int)(trap_Milliseconds()-startTime)/1000), AVERAGE_DISTANCE );

#endif //__SLOW_PATHING__

	PATHING_IGNORE_FRAME_TIME = qfalse;
}