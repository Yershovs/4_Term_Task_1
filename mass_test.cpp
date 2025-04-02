/**
 * @file mass_test.cpp
 * @brief Массовое тестирование сервера выпуклой оболочки
 */

#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <random>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include "convex_hull.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define NUM_CLIENTS 100
#define POINTS_PER_TEST 1000
#define TEST_LOG_FILE "tests.log"
#define MIN_COORD -1000
#define MAX_COORD 1000

// Хеш-функция для Point
struct PointHash {
    size_t operator()(const Point& p) const {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
};

// Сравнение точек
struct PointEqual {
    bool operator()(const Point& a, const Point& b) const {
        return a.x == b.x && a.y == b.y;
    }
};

using PointSet = std::unordered_set<Point, PointHash, PointEqual>;

/**
 * @brief Генерирует уникальные точки
 * @param count Количество точек
 * @param gen Генератор случайных чисел
 * @param dist Распределение
 * @return Вектор уникальных точек
 */
std::vector<Point> generateUniquePoints(int count, 
    std::mt19937& gen,
    std::uniform_int_distribution<>& dist) {
PointSet unique_points;
while (unique_points.size() < static_cast<size_t>(count)) {  // Исправлено здесь
unique_points.insert({dist(gen), dist(gen)});
}
return std::vector<Point>(unique_points.begin(), unique_points.end());
}

/**
 * @brief Конвертирует точки в строку
 * @param points Вектор точек
 * @return Строковое представление
 */
std::string pointsToString(const std::vector<Point>& points) {
    std::ostringstream oss;
    for (const auto& p : points) {
        oss << p.x << "," << p.y << " ";
    }
    return oss.str();
}

/**
 * @brief Сохраняет тест в файл
 * @param points Исходные точки
 * @param hull Точки оболочки
 */
void saveTestToFile(const std::vector<Point>& points,
                   const std::vector<Point>& hull) {
    std::ofstream out(TEST_LOG_FILE, std::ios::app);
    out << "POINTS:";
    for (const auto& p : points) {
        out << " " << p.x << "," << p.y;
    }
    out << "\nHULL:";
    for (const auto& p : hull) {
        out << " " << p.x << "," << p.y;
    }
    out << "\n---\n";
}

/**
 * @brief Выполняет тестовый запрос
 * @param testPoints Точки для теста
 * @param clientId ID клиента
 * @return Результат теста (true - успех)
 */
bool testClient(const std::vector<Point>& testPoints, int clientId) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Client " << clientId << ": WSAStartup failed\n";
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Client " << clientId << ": Socket error\n";
        WSACleanup();
        return false;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Client " << clientId << ": Connection failed\n";
        closesocket(sock);
        WSACleanup();
        return false;
    }

    std::string input = pointsToString(testPoints);
    std::vector<Point> localHull = convexHull(testPoints);
    saveTestToFile(testPoints, localHull);

    if (send(sock, input.c_str(), input.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Client " << clientId << ": Send failed\n";
        closesocket(sock);
        WSACleanup();
        return false;
    }

    std::vector<char> buffer(BUFFER_SIZE);
    std::string serverResponse;
    int bytesReceived;

    while ((bytesReceived = recv(sock, buffer.data(), buffer.size(), 0)) > 0) {
        serverResponse.append(buffer.data(), bytesReceived);
        if (bytesReceived < static_cast<int>(buffer.size())) break;
    }

    bool success = (serverResponse == pointsToString(localHull));
    closesocket(sock);
    WSACleanup();
    return success;
}

int main() {
    std::ofstream(TEST_LOG_FILE, std::ios::trunc).close();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(MIN_COORD, MAX_COORD);

    int passed = 0;
    for (int i = 0; i < NUM_CLIENTS; ++i) {
        auto points = generateUniquePoints(POINTS_PER_TEST, gen, dist);
        if (testClient(points, i)) passed++;
        std::cout << "Test " << i+1 << "/" << NUM_CLIENTS 
                  << " | Passed: " << passed << "\r";
    }

    std::cout << "\nTests completed: " << passed << "/" << NUM_CLIENTS 
              << " (" << (passed*100/NUM_CLIENTS) << "%)\n";
    return 0;
}