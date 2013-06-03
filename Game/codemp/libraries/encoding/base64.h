/******************************************\
*
*  Base-128 Encoding Algorithm
*
*  Encodes data into base-64 and back
*  The encoder supports both the official encoding scheme
*  as well as a URL-safe encoding scheme where the
*  + / and = symbols are replaced by - _ and . repectively
*
*  NOTE: This encoder does not follow the 76 char line length limitation!
*
*  Copyright (c) 2010 Lourens "BobaFett" Elzinga
*
\******************************************/

#ifndef base64__h
#define base64__h

#ifdef __cplusplus
extern "C"
{
#endif

/***********************
|*
|* Base64_EncodeLength
|*
|* Description: Returns the length of the buffer required to 
|*  encode binary data with the given length (in bytes).
|*  This includes the NULL terminator
|*
|* Arguments:
|* - length: Length of the binary data to encode
|*
|* Return value:
|* Size of the buffer required to store base-64 encoded string
|*
\*/
unsigned int Base64_EncodeLength(unsigned int length);

/***********************
|*
|* Base64_DecodeLength
|*
|* Description: Returns the length of the buffer required to 
|*  decode a base-64 encoded string in (in bytes)
|*
|* Arguments:
|* - length: Length of the base-64 encoded string. [Optional]
|* - base64: The base64 encoded string. [Optional]
|* - urlencoded: Whether this base-64 string uses the URL-safe encoding (where + / and = are replaced by - _ and .)
|*
|* Return value:
|* Size of the buffer required to store the decoded data
|*
|* Notes: Because base64 uses padding, you can either pass the length
|* of the string or the string itself. If the string is provided, it will
|* be checked for padding and the actual decode length is returned.
|* If the string is not provided, the decode length will be calculated based
|* on the provided length (which can be up to 2 bytes more than the actual length)
|*
|* If the string is passed and length is 0, strlen will be used to determine the length;
|*
\*/
unsigned int Base64_DecodeLength(unsigned int length, const char *base64, int urlencoded);

/***********************
|*
|* Base64_Encode
|*
|* Description: Encodes the provided data into base-64
|*
|* Arguments:
|* - data: Pointer to the data to encode
|* - length: The size of the buffer pointed to by data
|* - base64: Pointer to the buffer to store the base-64 encoded string in
|* - base64len: Length of the buffer pointed to by base64
|*
|* Return value:
|* 1 if the data was encoded successfully, 0 otherwise.
|*
|* Reasons for failure:
|* * NULL pointers passed to data and/or base64
|* * Insufficient space in base64 buffer
|*
|* NOTE: Use Base64_EncodeLength to determine the buffer size for base64. This size includes the null pointer.
|*       The encoder will automatically NULL-terminate the base64 buffer.
|*
|* The URL variant does the same, except that it encodes it in a URL safe variant of base64, where the + / and = characters
|* are replaced by - _ and . respectively
\*/
int Base64_Encode(const char *data, unsigned int length, char *base64, unsigned int base64len);
int Base64_EncodeURL(const unsigned char *data, unsigned int length, char *base64, unsigned int base64len);

/***********************
|*
|* Base64_Decode
|*
|* Description: Decodes the provided base-64 into data
|*
|* Arguments:
|* - base64: Pointer to the base-64 to encode
|* - b64len: Length of the base-64 string. Pass 0 (NULL) to determine this automatically.
|* - data: Pointer to the buffer to store the decoded data in
|* - length: Size of the buffer pointed to by data
|*
|* Return value:
|* 1 if the data was decoded successfully, 0 otherwise.
|*
|* Reasons for failure:
|* * NULL pointers passed to data and/or base64
|*
|* NOTE: Use Base64_DecodeLength to determine the buffer size for data.
|*       If the buffer is too small, the decoding will be stopped once the buffer is full.
|*
|*       Invalid characters are ignored by the decoder and are treated as 0-value characters.
|*
|* WARNING: If you provide the length of the base64 string, do NOT include the null terminator!
|*
|* The URL variant does the same, except that it encodes it in a URL safe variant of base64, where the + / and = characters
|* are replaced by - _ and . respectively
\*/
int Base64_Decode(const char *base64, unsigned int b64len, char *data, unsigned int length);
int Base64_DecodeURL(const char *base64, unsigned int b64len, char *data, unsigned int length);


#ifdef __cplusplus
}
#endif

#endif