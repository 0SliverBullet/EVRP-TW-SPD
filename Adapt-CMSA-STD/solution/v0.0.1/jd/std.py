import os
import numpy as np
import scipy.stats as stats

def read_sample(file_path):
    """
    从指定文件的最后 10 行读取数据（用逗号分隔，取第 1 列浮点数）。
    返回一个长度为 10 的浮点数列表。
    """
    if not os.path.exists(file_path):
        # 若文件不存在，可以选择返回 None 或抛出异常
        print(f"Warning: file not found -> {file_path}")
        return None
    
    with open(file_path, 'r') as f:
        lines = f.readlines()
    
    # 取最后 10 行数据
    data = []
    for line in lines[-10:]:
        # 假设每行是 "值,其他内容..."，只取第一个值
        try:
            value = float(line.strip().split(',')[0])
            data.append(value)
        except:
            pass  # 如果行格式异常，可忽略或处理
    
    return data if len(data) == 10 else None

def compute_stats(data):
    """
    给定一个浮点数列表 data，返回 (均值, 样本标准差)。
    如果 data 为 None 或长度不足，返回 None。
    """
    if not data or len(data) < 2:
        return None
    mean_val = np.mean(data)
    std_val = np.std(data, ddof=1)  # ddof=1 用于样本标准差
    return (mean_val, std_val)

# ======================
# 1) 小 timelimit 表格
# ======================
instances_small = [
    ("jd200_1", 1800),
    ("jd200_2", 1800),
    ("jd200_3", 1800),
    ("jd200_4", 1800),
    ("jd400_1", 5400),
    ("jd400_2", 5400),
    ("jd400_3", 5400),
    ("jd400_4", 5400),
    ("jd600_1", 9000),
    ("jd600_2", 9000),
    ("jd600_3", 9000),
    ("jd600_4", 9000),
    ("jd800_1", 12600),
    ("jd800_2", 12600),
    ("jd800_3", 12600),
    ("jd800_4", 12600),
    ("jd1000_1", 16200),
    ("jd1000_2", 16200),
    ("jd1000_3", 16200),
    ("jd1000_4", 16200)
]

# ======================
# 2) 大 timelimit 表格
# ======================
instances_large = [
    ("jd200_1", 3600),
    ("jd200_2", 3600),
    ("jd200_3", 3600),
    ("jd200_4", 3600),
    ("jd400_1", 10800),
    ("jd400_2", 10800),
    ("jd400_3", 10800),
    ("jd400_4", 10800),
    ("jd600_1", 18000),
    ("jd600_2", 18000),
    ("jd600_3", 18000),
    ("jd600_4", 18000),
    ("jd800_1", 25200),
    ("jd800_2", 25200),
    ("jd800_3", 25200),
    ("jd800_4", 25200),
    ("jd1000_1", 32400),
    ("jd1000_2", 32400),
    ("jd1000_3", 32400),
    ("jd1000_4", 32400)
]

def parse_instance_name(instance_name):
    """
    将类似 'jd200_1' -> 去掉 'jd' -> '200_1' -> 再 split('_') 得到 (n, subproblem)。
    返回 (n, subproblem) 两个字符串，供拼接文件名使用。
    """
    # 去掉前缀 jd
    name_str = instance_name[2:]  # "200_1"
    parts = name_str.split('_')
    if len(parts) == 2:
        n, sub = parts
        return n, sub
    else:
        return None, None

# 这里假设文件路径形如:
# ./small_timelimit/{n}_{sub}_timelimit={time}_subproblem=1.txt
# 或者你可以按需修改子问题编号或文件夹名称
def build_file_path_small(instance_name, timelimit):
    n, sub = parse_instance_name(instance_name)
    if not n or not sub:
        return None
    return f"./small_timelimit/{n}_{sub}_timelimit={timelimit}_subproblem=1.txt"

def build_file_path_large(instance_name, timelimit):
    n, sub = parse_instance_name(instance_name)
    if not n or not sub:
        return None
    return f"./large_timelimit/{n}_{sub}_timelimit={timelimit}_subproblem=1.txt"

# ================
# 主程序：逐一输出
# ================
print("===== 小 timelimit 结果 =====")
for (inst, tlim) in instances_small:
    file_path = build_file_path_small(inst, tlim)
    if file_path is None:
        print(f"{inst} (timelimit={tlim}): 文件名构造失败")
        continue
    
    data = read_sample(file_path)
    stats_val = compute_stats(data)
    if stats_val is None:
        print(f"{inst} (timelimit={tlim}): 数据不足或文件不存在")
    else:
        mean_val, std_val = stats_val
        print(f"{inst} (timelimit={tlim}): mean={mean_val:.2f}, std={std_val:.2f}")

print("\n===== 大 timelimit 结果 =====")
for (inst, tlim) in instances_large:
    file_path = build_file_path_large(inst, tlim)
    if file_path is None:
        print(f"{inst} (timelimit={tlim}): 文件名构造失败")
        continue
    
    data = read_sample(file_path)
    stats_val = compute_stats(data)
    if stats_val is None:
        print(f"{inst} (timelimit={tlim}): 数据不足或文件不存在")
    else:
        mean_val, std_val = stats_val
        print(f"{inst} (timelimit={tlim}): mean={mean_val:.2f}, std={std_val:.2f}")
