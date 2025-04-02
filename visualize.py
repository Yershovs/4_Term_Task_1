import matplotlib.pyplot as plt
import matplotlib.patches as patches
import re

def parse_test_file(filename):
    tests = []
    with open(filename, 'r') as f:
        content = f.read().split('---\n')
    
    for test in content:
        if not test.strip():
            continue
            
        points = []
        hull = []
        
        # Парсинг точек
        points_match = re.search(r'POINTS:((?:\s-?\d+,-?\d+)+)', test)
        if points_match:
            points_str = points_match.group(1).strip().split()
            points = [(float(p.split(',')[0]), float(p.split(',')[1])) for p in points_str]
        
        # Парсинг выпуклой оболочки
        hull_match = re.search(r'HULL:((?:\s-?\d+,-?\d+)+)', test)
        if hull_match:
            hull_str = hull_match.group(1).strip().split()
            hull = [(float(p.split(',')[0]), float(p.split(',')[1])) for p in hull_str]
        
        if points and hull:
            tests.append({'points': points, 'hull': hull})
    
    return tests

def visualize_test(test, index):
    fig, ax = plt.subplots(figsize=(10, 8))
    
    # Отображение всех точек
    x_pts = [p[0] for p in test['points']]
    y_pts = [p[1] for p in test['points']]
    ax.scatter(x_pts, y_pts, color='blue', label='Points')
    
    # Отображение выпуклой оболочки
    hull = test['hull']
    hull.append(hull[0])  # Замыкаем оболочку
    x_hull = [p[0] for p in hull]
    y_hull = [p[1] for p in hull]
    ax.plot(x_hull, y_hull, 'r-', label='Convex Hull')
    ax.fill(x_hull, y_hull, alpha=0.2, color='red')
    
    # Настройка графика
    ax.set_title(f'Test Case #{index + 1}')
    ax.set_xlabel('X coordinate')
    ax.set_ylabel('Y coordinate')
    ax.grid(True)
    ax.legend()
    ax.axis('equal')
    
    plt.tight_layout()
    plt.savefig(f'test_case_{index + 1}.png')
    plt.close()

def main():
    tests = parse_test_file('tests.log')
    
    print(f"Found {len(tests)} test cases")
    for i, test in enumerate(tests):
        visualize_test(test, i)
        print(f"Visualized test case {i + 1}")
    
    print("All visualizations saved as PNG files")

if __name__ == "__main__":
    main()