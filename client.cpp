#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <sstream>
#include "convex_hull.h"

#define PORT 8080 ///< Порт для подключения к серверу
#define BUFFER_SIZE 1024 ///< Начальный размер буфера

/**
 * @brief Разбирает строку с точками в вектор структур Point.
 * 
 * @param input Строка с точками в формате "x1,y1 x2,y2 ...".
 * @return std::vector<Point> Вектор точек.
 */
std::vector<Point> parsePoints(const std::string& input) {
    std::vector<Point> points;
    std::istringstream iss(input);
    int x, y;
    char sep;
    while (iss >> x >> sep >> y) {
        points.push_back({x, y});
        if (iss.peek() == ' ') iss.ignore();
    }
    return points;
}

/**
 * @brief Преобразует вектор точек в строку.
 * 
 * @param points Вектор точек.
 * @return std::string Строка в формате "x1,y1 x2,y2 ...".
 */
std::string pointsToString(const std::vector<Point>& points) {
    std::ostringstream oss;
    for (const auto& p : points) {
        oss << p.x << "," << p.y << " ";
    }
    return oss.str();
}

/**
 * @brief Основная функция клиента.
 * 
 * Создает сокет, подключается к серверу, отправляет точки и получает выпуклую оболочку.
 * Также выполняет локальные вычисления для проверки сервера.
 * 
 * @return int Код завершения программы (0 - успех, -1 - ошибка).
 */
int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); ///< Инициализация Winsock

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); ///< Создание сокета
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation error\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET; ///< Использование IPv4
    serv_addr.sin_port = htons(PORT); ///< Установка порта
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); ///< Установка IP-адреса сервера

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection Failed\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    std::cout << "Enter points in format 'x1,y1 x2,y2 ...': ";
    std::string input;
    std::getline(std::cin, input); ///< Получение ввода от пользователя

    ///< Локальное вычисление выпуклой оболочки
    std::vector<Point> localPoints = parsePoints(input);
    std::vector<Point> localHull = convexHull(localPoints);
    std::string localResult = pointsToString(localHull);

    send(sock, input.c_str(), input.size(), 0); ///< Отправка данных на сервер

    std::vector<char> buffer(BUFFER_SIZE); ///< Динамический буфер для приема данных
    std::string serverResponse;
    int bytesReceived;

    while ((bytesReceived = recv(sock, buffer.data(), buffer.size(), 0)) > 0) {
        serverResponse.append(buffer.data(), bytesReceived);
        if (bytesReceived < static_cast<int>(buffer.size())) {
            break; ///< Все данные получены
        }
    }

    ///< Проверка совпадения результатов
    if (serverResponse == localResult) {
        std::cout << "Results match!\n";
        std::cout << "Convex hull points: " << serverResponse << std::endl;
    } else {
        std::cerr << "Error: local and server results are different!\n";
        std::cerr << "Local result: " << localResult << std::endl;
        std::cerr << "Server result: " << serverResponse << std::endl;
    }

    closesocket(sock); ///< Закрытие сокета
    WSACleanup(); ///< Очистка Winsock
    return 0;
}