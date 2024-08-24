import os

def count_lines_in_file(file_path):
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as file:
        return sum(1 for _ in file)

def count_lines_in_folder(folder_path):
    total_lines = 0
    for root, _, files in os.walk(folder_path):
        for file in files:
            file_path = os.path.join(root, file)
            total_lines += count_lines_in_file(file_path)
    return total_lines

total_lines = 0
total_lines += count_lines_in_folder("src/basic")
total_lines += count_lines_in_folder("src/auslander-parter")
total_lines += count_lines_in_folder("src/sefe")

total_lines += count_lines_in_file("src/main.cpp")

print(f'Total number of lines in all files: {total_lines}')
