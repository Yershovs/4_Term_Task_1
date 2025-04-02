/**
 * @file convex_hull.h
 * @brief Алгоритм построения выпуклой оболочки
 */

 #include <vector>
 #include <algorithm>
 
 /**
  * @brief Структура точки в 2D пространстве
  */
 struct Point {
     int x; ///< Координата X
     int y; ///< Координата Y
 };
 
 /**
  * @brief Определяет ориентацию тройки точек
  * @param p Первая точка
  * @param q Вторая точка
  * @param r Третья точка
  * @return 0 - коллинеарны, 1 - по часовой, 2 - против часовой
  */
 inline int orientation(const Point& p, const Point& q, const Point& r) {
     int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
     return (val == 0) ? 0 : ((val > 0) ? 1 : 2);
 }
 
 /**
  * @brief Вычисляет квадрат расстояния между точками
  * @param p1 Первая точка
  * @param p2 Вторая точка
  * @return Квадрат расстояния
  */
 inline int distanceSq(const Point& p1, const Point& p2) {
     return (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);
 }
 
 /**
  * @brief Находит выпуклую оболочку (не модифицирует входные данные)
  * @param points Входные точки (const ссылка)
  * @return Точки выпуклой оболочки
  */
 inline std::vector<Point> convexHull(const std::vector<Point>& points) {
     if (points.size() < 3) return points;
     
     // Создаем рабочую копию
     std::vector<Point> working_copy = points;
     size_t n = working_copy.size();
     
     // Находим самую нижнюю левую точку
     size_t min_idx = 0;
     for (size_t i = 1; i < n; i++) {
         if (working_copy[i].y < working_copy[min_idx].y || 
            (working_copy[i].y == working_copy[min_idx].y && 
             working_copy[i].x < working_copy[min_idx].x)) {
             min_idx = i;
         }
     }
     std::swap(working_copy[0], working_copy[min_idx]);
     Point p0 = working_copy[0];
     
     // Сортируем точки по полярному углу
     std::sort(working_copy.begin() + 1, working_copy.end(), 
         [&p0](const Point& p1, const Point& p2) {
             int o = orientation(p0, p1, p2);
             return (o == 2) || (o == 0 && distanceSq(p0, p1) < distanceSq(p0, p2));
         });
     
     // Строим оболочку
     std::vector<Point> hull;
     hull.push_back(working_copy[0]);
     hull.push_back(working_copy[1]);
     
     for (size_t i = 2; i < n; i++) {
         while (hull.size() > 1 && 
                orientation(hull[hull.size()-2], hull.back(), working_copy[i]) != 2) {
             hull.pop_back();
         }
         hull.push_back(working_copy[i]);
     }
     
     return hull;
 }