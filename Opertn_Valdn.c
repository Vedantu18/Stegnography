#include "Header.h"

/******************************************************************************************************************/
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    // printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    // printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
/******************************************************************************************************************/
/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    uint size = (uint)ftell(fptr);
    rewind(fptr);
    return size;
}
/******************************************************************************************************************/

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/******************************************************************************************************************/

// operation check function definition
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if (strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

/******************************************************************************************************************/

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2];
        printf("Original image %s\n", encInfo->src_image_fname);
    }
    else
    {
        return e_failure;
    }

    if (strstr(argv[3], ".txt") != NULL)
    {
        encInfo->secret_fname = argv[3];
        printf("Secret File name %s\n", encInfo->secret_fname);
        strncpy(encInfo->extn_secret_file, ".txt", MAX_FILE_SUFFIX);
    }
    else
    {
        return e_failure;
    }
    if (strstr(argv[4], ".bmp") != NULL)
    {
        encInfo->stego_image_fname = argv[4];
        printf("Stego file name %s\n", encInfo->stego_image_fname);
    }
    else
    {
        encInfo->stego_image_fname = "stego_img.bmp";
        return e_success;
    }
}

/******************************************************************************************************************/

/* check capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    // Test get_image_size_for_bmp
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    printf("INFO: Image size = %u\n", encInfo->image_capacity);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    uint required_capacity = (encInfo->size_secret_file * 8) + 54 + 32; // 54-> header, 32-> metadate-> file size, file extension, magic string, reserved
    if (encInfo->image_capacity > required_capacity)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
/******************************************************************************************************************/
/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char header[54];

    if (fread(header, 1, 54, fptr_src_image) != 54)
    {
        return e_failure;
    }

    if (fwrite(header, 1, 54, fptr_dest_image) != 54)
    {
        return e_failure;
    }

    return e_success;
}
/******************************************************************************************************************/

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(unsigned char data, char *image_buffer)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        unsigned char bit = (data >> (7 - i)) & 1;

        image_buffer[i] = image_buffer[i] & 0xFE; // clearing last bit of image buffering[i] 0.

        image_buffer[i] = image_buffer[i] | bit; // add secret bit into image byte
    }
    return e_success;
}
/******************************************************************************************************************/

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    unsigned char buffer[8];

    for (int i = 0; magic_string[i] != '\0'; i++)
    {
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], buffer);
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}
/******************************************************************************************************************/

/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    unsigned char buffer[8];

    for (int i = 0; file_extn[i] != '\0'; i++)
    {
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}
/******************************************************************************************************************/

Status encode_size_to_lsb(unsigned long size, unsigned char *image_buffer)
{
    int i;
    for (i = 0; i < 32; i++)
    {
        unsigned char bit = (size >> (31 - i)) & 1;

        image_buffer[i] = image_buffer[i] & 0xFE; // clearing last bit of image buffering[i] 0.

        image_buffer[i] = image_buffer[i] | bit; // add secret bit into image byte
    }
    return e_success;
}

/******************************************************************************************************************/

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    unsigned char buffer[32];

    fread(buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(file_size, buffer);
    fwrite(buffer, 1, 32, encInfo->fptr_stego_image);

    return e_success;
}
/******************************************************************************************************************/

/* Encode secret file extension size */
Status encode_secret_file_extn_size(EncodeInfo *encInfo)
{
    unsigned char buffer[32];
    unsigned long extn_size = strlen(encInfo->extn_secret_file);

    fread(buffer, 1, 32, encInfo->fptr_src_image);
    encode_size_to_lsb(extn_size, buffer);
    fwrite(buffer, 1, 32, encInfo->fptr_stego_image);

    return e_success;
}
/******************************************************************************************************************/
/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{


}

/******************************************************************************************************************/

/* Perform the encoding Operation*/
Status do_encoding(EncodeInfo *encInfo)
{

    //  open_file function calling
    if (open_files(encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Open_File");
        return e_failure;
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "Open_File");
    }

    // check capacity function calling
    if (check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Check_Capacity");
        return e_failure;
    }
    {
        printf("SUCCESS: %s function completed\n", "Check_Capacity");
    }

    // function calling for copy bmp header file
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Copy_BMP_Header");
        return e_failure;
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "Copy_BMP_Header");
    }

    // function calling for the encode magic string
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Encoding_Magic_String");
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "Encoding_Magic_String");
    }
    // function calling for the encode secret file extension size
    if (encode_secret_file_extn_size(encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "encoding secret file extencion size");
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "encoding secret file extencion size");
    }

    // function calling for the encoding the secret file extension
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Encode_Extension_Sectret_File");
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "Encoding_Magic_String");
    }

    // function calling for the encoding secret file size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
    {
        printf("ERROR: %s function failed\n", "Encoding_Secret_File_Size");
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "Encoding_Secret_File_Size");
    }

    // function calling for the secret file data
    if (encode_secret_file_data(encInfo) == e_failure)
    {
        printf("ERROR: %s fucntion failed\n", "encode secret file data");
    }
    else
    {
        printf("SUCCESS: %s function completed\n", "encode secret file data");

    }
}
