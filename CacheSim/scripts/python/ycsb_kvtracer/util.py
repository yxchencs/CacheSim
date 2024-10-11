import re


def extract_disk_size(str):
    # 使用正则表达式提取数字和单位
    match = re.match(r'(\d+(\.\d+)?)\s*([a-zA-Z]+)', str)
    if match:
        number = match.group(1)
        unit = match.group(3)
        # print(f'number:{number}, unit: {unit}')
        if unit == 'GB':
            return float(number) * 1024 * 1024 * 1024
        elif unit == 'MB':
            return float(number) * 1024 * 1024
    else:
        return None
