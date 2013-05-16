/******************************************\
*
*  Base-128 Encoding Algorithm
*
*  Encodes data into base-128 and back
*
*  The encoder will not use characters with special meaning (such as % \ and ") or any non-printable or whitespace characters
*
*  When decoding, invalid characters will be treated as if they represent the value 0
*
*  Copyright (c) 2010 Lourens "BobaFett" Elzinga
*
\******************************************/

#ifndef base128__h
#define base128__h

#ifdef __cplusplus
extern "C"
{
#endif

/***********************
|*
|* Base128_EncodeLength
|*
|* Description: Returns the length of the buffer required to 
|*  encode binary data with the given length (in bytes).
|*  This includes the NULL terminator
|*
|* Arguments:
|* - length: Length of the binary data to encode
|*
|* Return value:
|* Size of the buffer required to store base-128 encoded string
|*
\*/
unsigned int Base128_EncodeLength(unsigned int length);

/***********************
|*
|* Base128_DecodeLength
|*
|* Description: Returns the length of the buffer required to 
|*  decode a base-128 encoded string in (in bytes)
|*
|* Arguments:
|* - length: Length of the base-128 encoded string
|*
|* Return value:
|* Size of the buffer required to store the decoded data
|*
\*/
unsigned int Base128_DecodeLength(unsigned int length);

/***********************
|*
|* Base128_Encode
|*
|* Description: Encodes the provided data into base-128
|*
|* Arguments:
|* - data: Pointer to the data to encode
|* - length: The size of the buffer pointed to by data
|* - base128: Pointer to the buffer to store the base-128 encoded string in
|* - base128len: Length of the buffer pointed to by base128
|*
|* Return value:
|* 1 if the data was encoded successfully, 0 otherwise.
|*
|* Reasons for failure:
|* * NULL pointers passed to data and/or base128
|* * Insufficient space in base128 buffer
|*
|* NOTE: Use Base128_EncodeLength to determine the buffer size for base128. This size includes the null pointer.
|*       The encoder will automatically NULL-terminate the base128 buffer.
|*
\*/
int Base128_Encode(const void *data, unsigned int length, char *base128, unsigned int base128len);

/***********************
|*
|* Base128_Decode
|*
|* Description: Decodes the provided base-128 into data
|*
|* Arguments:
|* - base128: Pointer to the base-128 to encode
|* - b128len: Length of the base-128 string. Pass 0 (NULL) to determine this automatically.
|* - data: Pointer to the buffer to store the decoded data in
|* - length: Size of the buffer pointed to by data
|*
|* Return value:
|* 1 if the data was decoded successfully, 0 otherwise.
|*
|* Reasons for failure:
|* * NULL pointers passed to data and/or base128
|*
|* NOTE: Use Base128_DecodeLength to determine the buffer size for data.
|*       If the buffer is too small, the decoding will be stopped once the buffer is full.
|*
|*       Invalid characters are ignored by the decoder and are treated as 0-value characters.
|*
|* WARNING: If you provide the length of the base128 string, do NOT include the null terminator!
\*/
int Base128_Decode(const char *base128, unsigned int b128len, void *data, unsigned int length );


#ifdef __cplusplus
}
#endif

#endif