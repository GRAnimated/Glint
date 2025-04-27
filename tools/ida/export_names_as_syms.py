import ida_kernwin
import idautils
import idc

remove_list = ("sub_", "off_", "nullsub", "j_")

output_file_path = ida_kernwin.ask_file(1, "*.txt", "Save symbol file")

if output_file_path:
    with open(output_file_path, 'w') as f:
        f.write("@mc:1017\n\n")
        for func_addr in idautils.Functions():
            func_name = idc.get_func_name(func_addr)
            if func_name and not func_name.startswith(remove_list) and ":" not in func_name:
                f.write("{} = {}\n".format(func_name, hex(func_addr - 0x7100000000)))

        for segea in idautils.Segments():
            for ea in idautils.Heads(segea, idc.get_segm_end(segea)):
                if idc.get_segm_name(ea) == ".data" or idc.get_segm_name(ea) == ".bss":
                    name = idc.get_name(ea)
                    if name and not name.startswith(remove_list) and ":" not in name:
                        f.write("{} = {}\n".format(name, hex(ea - 0x7100000000)))

    print(f"Symbol file '{output_file_path}' has been generated.")
else:
    print("Operation cancelled by the user.")
