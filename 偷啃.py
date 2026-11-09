import os
import struct
import win32api
import win32con

BUFFER_BYTES = 1024
MAGIC_SRC = "__MAGIC_ORIGINAL_EXE_PATH_BUFFER_0123456789ABCDEF_PLACEHOLDER__".encode("utf-16le")
MAGIC_DST = "__MAGIC_TARGET_LINK_PATH_BUFFER_0123456789ABCDEF_PLACEHOLDER__".encode("utf-16le")

偷 = {
    r"D:\Program Files\Microsoft VS Code\Code.exe": "微软大战代码",
}


def 同步子系统(原路径, data):
    with open(原路径, "rb") as f:
        f.seek(0x3C)
        e_lfanew_src = struct.unpack("<I", f.read(4))[0]
        f.seek(e_lfanew_src + 24 + 68)
        subsystem = struct.unpack("<H", f.read(2))[0]

    e_lfanew_dst = struct.unpack_from("<I", data, 0x3C)[0]
    struct.pack_into("<H", data, e_lfanew_dst + 24 + 68, subsystem)


def 复制图标(源路径, 目标路径):
    h = win32api.LoadLibraryEx(源路径, 0, win32con.LOAD_LIBRARY_AS_DATAFILE)
    try:
        group_names = win32api.EnumResourceNames(h, win32con.RT_GROUP_ICON)
        groups = [(g, win32api.LoadResource(h, win32con.RT_GROUP_ICON, g)) for g in group_names]
        icon_names = win32api.EnumResourceNames(h, win32con.RT_ICON)
        icons = [(i, win32api.LoadResource(h, win32con.RT_ICON, i)) for i in icon_names]
    except Exception:
        return
    finally:
        win32api.FreeLibrary(h)

    h_update = win32api.BeginUpdateResource(目标路径, False)
    for g, data in groups:
        win32api.UpdateResource(h_update, win32con.RT_GROUP_ICON, g, data)
    for i, data in icons:
        win32api.UpdateResource(h_update, win32con.RT_ICON, i, data)
    win32api.EndUpdateResource(h_update, False)


def 生成启动器():
    template_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "启动器.exe")
    with open(template_path, "rb") as f:
        template_bytes = f.read()

    p_src = template_bytes.find(MAGIC_SRC)
    p_dst = template_bytes.find(MAGIC_DST)

    for 原程序, 别名 in 偷.items():
        原程序路径 = os.path.abspath(原程序)
        if not 别名.endswith(".exe"):
            别名 += ".exe"

        目标目录 = os.path.dirname(原程序路径)
        硬链接路径 = os.path.join(目标目录, 别名)
        启动器路径 = os.path.join(目标目录, f"启动_{别名}")

        data = bytearray(template_bytes)

        src_bytes = (原程序路径.encode("utf-16le") + b"\x00\x00").ljust(BUFFER_BYTES, b"\x00")
        dst_bytes = (硬链接路径.encode("utf-16le") + b"\x00\x00").ljust(BUFFER_BYTES, b"\x00")

        data[p_src : p_src + BUFFER_BYTES] = src_bytes
        data[p_dst : p_dst + BUFFER_BYTES] = dst_bytes

        同步子系统(原程序路径, data)

        with open(启动器路径, "wb") as f:
            f.write(data)

        复制图标(原程序路径, 启动器路径)

        print(f"已生成启动器: {启动器路径}")


if __name__ == "__main__":
    生成启动器()
