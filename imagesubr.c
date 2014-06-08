#include <imlib2.h>

#include "imageutils.h"

// return a string "explainig" the reason a load_image (or save_image)
// failed.
char *
image_decode_load_error(Imlib_Load_Error error) {
    switch (error) {
    case IMLIB_LOAD_ERROR_NONE:
        return "imlib_load_error_none";
    case IMLIB_LOAD_ERROR_FILE_DOES_NOT_EXIST:
        return "imlib_load_error_file_does_not_exist";
    case IMLIB_LOAD_ERROR_FILE_IS_DIRECTORY:
        return "imlib_load_error_file_is_directory";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_READ:
        return "imlib_load_error_permission_denied_to_read";
    case IMLIB_LOAD_ERROR_NO_LOADER_FOR_FILE_FORMAT:
        return "imlib_load_error_no_loader_for_file_format";
    case IMLIB_LOAD_ERROR_PATH_TOO_LONG:
        return "imlib_load_error_path_too_long";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NON_EXISTANT:
        return "imlib_load_error_path_component_non_existant";
    case IMLIB_LOAD_ERROR_PATH_COMPONENT_NOT_DIRECTORY:
        return "imlib_load_error_path_component_not_directory";
    case IMLIB_LOAD_ERROR_PATH_POINTS_OUTSIDE_ADDRESS_SPACE:
        return "imlib_load_error_path_points_outside_address_space";
    case IMLIB_LOAD_ERROR_TOO_MANY_SYMBOLIC_LINKS:
        return "imlib_load_error_too_many_symbolic_linksn";
    case IMLIB_LOAD_ERROR_OUT_OF_MEMORY:
        return "imlib_load_error_out_of_memory";
    case IMLIB_LOAD_ERROR_OUT_OF_FILE_DESCRIPTORS:
        return "imlib_load_error_out_of_file_descriptors";
    case IMLIB_LOAD_ERROR_PERMISSION_DENIED_TO_WRITE:
        return "imlib_load_error_permission_denied_to_write";
    case IMLIB_LOAD_ERROR_OUT_OF_DISK_SPACE:
        return "imlib_load_error_out_of_disk_space";
    case IMLIB_LOAD_ERROR_UNKNOWN:
        return "imlib_load_error_unknown";
    default:
        return "unknown/invalid error";
    }
}
