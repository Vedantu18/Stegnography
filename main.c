#include "Header.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;

    OperationType operation = check_operation_type(argv);

    if (operation == e_encode)
    {

        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR: Invalid encode arguments\n");
            printf("Usage: %s -e <secret.txt> <image.bmp> <output.bmp>\n", argv[0]);
            return 1;
        }

        printf("Started Encoding Process\n");

        if (do_encoding(&encInfo) == e_failure)
        {
            printf("ERROR: Encoding Failed\n");
            return 1;
        }

        printf("Encoding successful! ✓\n");
    }
    else if (operation == e_decode)
    {
        if (read_and_validate_decode_args(argv, &encInfo) == e_failure)
        {
            printf("ERROR: Invalid decode arguments\n");
            printf("Usage: %s -d <stego_image.bmp> <output.txt>\n", argv[0]);
            return 1;
        }

        printf("Started Decoding Process\n");

        if (do_decoding(&encInfo) == e_failure)
        {
            printf("ERROR: Decoding Failed\n");
            return 1;
        }

        printf("Decoding successful! ✓\n");
    }
    else if (operation == e_unsupported)
    {
        printf("ERROR: Unsupported operation\n");
        printf("  Encode: %s -e <secret.txt> <image.bmp> <output.bmp>\n", argv[0]);
        printf("  Decode: %s -d <stego_image.bmp> <output.txt>\n", argv[0]);
        return 1;
    }
    else
    {
        printf("ERROR: Invalid arguments\n");
        return 1;
    }

    return 0;
}