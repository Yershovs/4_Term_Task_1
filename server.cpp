#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <sstream>
#include "convex_hull.h"

#define PORT 8080 ///< Порт, на котором сервер ожидает подключения
#define BUFFER_SIZE 1024 ///< Начальный размер буфера
#define MAX_QUEUE 1000 ///< Максимальное количество клиентов в очереди

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
 * @brief Основная функция сервера.
 * 
 * Создает сокет, ожидает подключения, обрабатывает запросы и отправляет результаты.
 * 
 * @return int Код завершения программы (0 - успех, -1 - ошибка).
 */
int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); ///< Инициализация Winsock

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0); ///< Создание сокета
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return -1;
    }

    sockaddr_in address;
    address.sin_family = AF_INET; ///< Использование IPv4
    address.sin_addr.s_addr = INADDR_ANY; ///< Принимать подключения на все IP-адреса
    address.sin_port = htons(PORT); ///< Установка порта

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    if (listen(server_fd, MAX_QUEUE) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr;
        int addrlen = sizeof(client_addr);
        SOCKET new_socket = accept(server_fd, (sockaddr*)&client_addr, &addrlen); ///< Принятие подключения
        if (new_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed\n";
            continue;
        }

        std::vector<char> buffer(BUFFER_SIZE); ///< Динамический буфер для приема данных
        std::string request;
        int bytesReceived;

        while ((bytesReceived = recv(new_socket, buffer.data(), buffer.size(), 0)) > 0) {
            request.append(buffer.data(), bytesReceived);
            if (bytesReceived < static_cast<int>(buffer.size())) {
                break; ///< Все данные получены
            }
        }

        std::cout << "Received points: " << request << std::endl;

        std::vector<Point> points = parsePoints(request);
        std::vector<Point> hull = convexHull(points);
        std::string response = pointsToString(hull);

        send(new_socket, response.c_str(), response.size(), 0); ///< Отправка результата клиенту
        closesocket(new_socket); ///< Закрытие сокета клиента
    }

    closesocket(server_fd); ///< Закрытие серверного сокета
    WSACleanup(); ///< Очистка Winsock
    return 0;
}