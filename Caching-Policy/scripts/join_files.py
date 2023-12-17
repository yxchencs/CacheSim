# 本脚本用于拼接指定路径directory_path中的所有txt文件内容，合并后不会多出额外的空行，文件按文件名字典序排列

import os

# 指定目录的路径
directory_path = 'D:/Projects/Caching-Policy/Caching-Policy/trace/WorkSpace_nexus5/after'

# 创建一个用于存储所有文本内容的列表
all_text = ["offset,size,type\n"]

# 遍历目录下的所有文件
for filename in os.listdir(directory_path):
    if filename.endswith('.txt'):  # 仅处理txt文件
        print(filename)
        file_path = os.path.join(directory_path, filename)
        with open(file_path, 'r') as file:
            file_contents = file.read()
            # # 删除最后一个换行符
            # if file_contents.endswith('/n'):
            #     file_contents = file_contents[:-1]
            all_text.append(file_contents)

# 将所有文本内容拼接成一个字符串
concatenated_text = ''.join(all_text)

# 指定输出文件的路径
output_file_path = 'D:/Projects/Caching-Policy/Caching-Policy/trace/WorkSpace_nexus5/nexus5_31.txt'

# 将拼接后的文本内容写入输出文件
with open(output_file_path, 'w') as output_file:
    output_file.write(concatenated_text)

print(f"合并后的文件已保存到: {output_file_path}")
