
	/**************************************************
	* DetourPatcher by Deathspike
	*
	* A generic patcher class created to simplify the
	* detours and patches being placed for JKG. It will
	* automatically create trampolines, allow inline
	* patches, and more.
	**************************************************/

	#define MakeDelta( cast, x, y )		( cast )(( unsigned long )( x ) - ( unsigned long )( y ))
	#define MakePtr( cast, x, y )		( cast )(( unsigned long )( x ) + ( unsigned long )( y ))

	unsigned long						Attach( unsigned long dwAddress, unsigned long dwAddressTarget );
	void								AttachClean( unsigned long dwAddress, unsigned long dwAddressTarget, unsigned long *dwAddressTrampoline );
	void								DisAssemble( unsigned char *iptr0, unsigned long *osizeptr );
	unsigned long						Detach( unsigned long pAddress, unsigned long pTramp );
	unsigned long						GetLen( unsigned long Address );
	unsigned long						GetTramp( unsigned long pAddress, unsigned int iLen );
	unsigned long						InlineFetch( unsigned long pAddress, unsigned int iLen );
	unsigned long						InlinePatch( unsigned long pAddress, unsigned long pNewAddress, unsigned int iLen );
	void								Patch( unsigned long pAddress, unsigned char bByte );
	void								ReProtect( unsigned long pAddress, unsigned int iLen );
	void								UnProtect( unsigned long pAddress, unsigned int iLen );
