#include "commandlist.h"

#ifdef KOPE_DIRECT3D12
#include <kope/direct3d12/commandlist_functions.h>
#endif

#ifdef KOPE_VULKAN
#include <kope/vulkan/commandlist_functions.h>
#endif

void kope_g5_command_list_begin_render_pass(kope_g5_command_list *list, const kope_g5_render_pass_parameters *parameters) {
	KOPE_G5_CALL2(command_list_begin_render_pass, list, parameters);
}

void kope_g5_command_list_end_render_pass(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_end_render_pass, list);
}

void kope_g5_command_list_present(kope_g5_command_list *list) {
	KOPE_G5_CALL1(command_list_present, list);
}

void kope_g5_command_list_set_index_buffer(kope_g5_command_list *list, kope_g5_buffer *buffer, kope_g5_index_format index_format, uint64_t offset,
                                           uint64_t size) {
	KOPE_G5_CALL5(command_list_set_index_buffer, list, buffer, index_format, offset, size);
}
