#pragma once

#include <QWebSocket>


class Connection
{
public:
    Connection(QWebSocket*  _pClientSocket);
    QWebSocket*  pClientSocket;
};

