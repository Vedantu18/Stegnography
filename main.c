#include "Header.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    uint img_size;

    // Fill with sample filenames
    // encInfo.src_image_fname = "beautiful.bmp";
    // encInfo.secret_fname = "secret.txt";
    // encInfo.stego_image_fname = "stego_img.bmp";

    // Checking oprtation is going on here
    if(check_operation_type(argv) == e_encode)
    {
        read_and_validate_encode_args(argv, &encInfo);
    }
    else if(check_operation_type(argv) == e_decode)
    {
        read_and_validate_encode_args(argv, &encInfo);
    }
    else
    {
        return e_unsupported;
    }
    printf("Started Encoding Process\n");

    // // Test get_image_size_for_bmp
    // img_size = get_image_size_for_bmp(encInfo.fptr_src_image);
    // printf("INFO: Image size = %u\n", img_size);

    if(do_encoding(&encInfo) == e_failure)
    {
        printf("Invalid Encode arguments\n");
        return e_failure;
    }
    printf("Encoding successful\n");


    return 0;
}
