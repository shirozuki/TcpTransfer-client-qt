#include "client.h"

Client::Client(QObject *parent) : QObject(parent)
{

}

void Client::selectionMenu()
{
    int selection;
    QByteArray srvResponse;

    while(true)
    {
        selection = 0;
        srvResponse.clear();

        std::cout << "\n------------- MENU GŁÓWNE -------------\n"
                     "1. Pobierz plik z serwera,\n"
                     "2. Wyświetl pliki dostępne na serwerze,\n"
                     "3. Zakończ program.\n"
                     "=======================================\n";

        std::cout << ">> ";
        std::cin >> selection;
        std::cout << "\n";

        switch(selection)
        {
        case 1:
            socket->write("1");
            socket->waitForBytesWritten(1000);
            socket->waitForReadyRead(1000);
            srvResponse = socket->readAll();
            if(srvResponse.data()[0] != '1')
            {
                std::cout << "Błąd serwera!\nKończenie pracy programu.\n\n";
                exit(EXIT_FAILURE);
            }
            else
            {
                std::cout << "Serwer przeszedł w tryb udostępniania plików\n\n";
                downloadData();
            }
            break;

        case 2:
            socket->write("2");
            socket->waitForBytesWritten(1000);
            socket->waitForReadyRead(1000);
            srvResponse = socket->readAll();
            if(srvResponse.data()[0] != '1')
            {
                std::cout << "Błąd serwera!\nKończenie pracy programu.\n";
                exit(EXIT_FAILURE);
            }
            else
            {
                std::cout << "Serwer przeszedł w tryb listowania katalogów\n";
                listFiles();
            }
            break;

        case 3:
            socket->write("3");
            socket->waitForBytesWritten();
            socket->close();
            std::cout << "\n* Do widzenia *\n";
            exit(EXIT_SUCCESS);
            break;

        default:
            continue;
        }
    }
}

void Client::downloadData()
{
    char fileName[128];
    QByteArray response;
    memset(fileName, 0, 128);

    std::cout << "Nazwa pliku: ";
    std::cin >> fileName;

    QFile fp(fileName);

    if(!fp.open(QFile::WriteOnly))
    {
        std::cerr << "Nie można utworzyć pliku docelowego!\n";
        socket->close();
        exit(EXIT_FAILURE);
    }

    socket->write(fileName);
    socket->waitForBytesWritten();

    socket->waitForReadyRead();
    response = socket->readAll();

    if(response.data()[0] == '1')
    {
        std::cout << "Plik został znaleziony na serwerze!\n";
    }

    else if(response.data()[0] == '0')
    {
        std::cout << "Plik nie został znaleziony na serwerze!\n\n";
        return;
    }

    else
    {
        std::cout << "Błąd wewnętrzny serwera. Kończenie pracy programu.\n";
        exit(EXIT_FAILURE);
    }

    qint64 fileSize, allReceivedData, sessionReceivedData, packetCount;
    char buffer[BUFFER_SIZE];
    QByteArray b_fileSize;

    fileSize = allReceivedData = sessionReceivedData = packetCount = 0;

    socket->waitForReadyRead();
    b_fileSize = socket->readAll();
    fileSize = std::atoll(b_fileSize.data());

    if(fileSize <= 0)
    {
        std::cout << "Serwer wysłał błędny rozmiar pliku!\n"
                  << "Wracam do Menu...\n";
        socket->write("N");
        socket->waitForBytesWritten();
        return;
    }

    else
    {
        socket->write("K");
        socket->waitForBytesWritten();
    }

    std::cout << "Rozmiar pliku: " << fileSize << " bajtów.\n";

    while(allReceivedData < fileSize)
    {
        packetCount++;
        memset(buffer, 0, BUFFER_SIZE);
        socket->waitForReadyRead();

        sessionReceivedData = socket->read(buffer, BUFFER_SIZE);

        if(sessionReceivedData <= 0)
        {
            std::cerr << "Błąd przy odbieraniu pliku!\n";
            break;
        }

        fp.write(buffer, sessionReceivedData);
        allReceivedData += sessionReceivedData;

        std::printf("\rPobrano %lld z %lld bajtów. Pakiet: %lld", allReceivedData, fileSize, packetCount);

        socket->write("K");
        socket->waitForBytesWritten();

    }

    if(allReceivedData == fileSize)
    {
        std::cout << "\nPlik Pobrany Poprawnie!\n";
        socket->close();
        exit(EXIT_SUCCESS);
    }

    else
    {
        std::cout << "Błąd przy pobieraniu pliku!\n";
        socket->close();
        exit(EXIT_FAILURE);
    }

}

void Client::listFiles()
{
    QByteArray list;

    std::cout << "\n*-*-*-*-* LISTA PLIKÓW *-*-*-*-*\n\n";

    while(socket->waitForReadyRead(1500))
    {
        list += socket->readAll();
    }

    std::cout << list.data();
    std::cout << "\n*-*-*-*-* KONIEC *-*-*-*-*\n\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    exit(EXIT_SUCCESS);
}

void Client::Connect()
{
    socket = new QTcpSocket(this);

    std::string ipAddr = "";
    long srvPort = 0;

    std::cout << "Adres IP serwera: ";
    std::cin >> ipAddr;
    std::cout << "Port: ";
    std::cin >> srvPort;
    std::cout << "\n";

    socket->connectToHost(QString::fromUtf8(ipAddr.c_str()), srvPort);

    if(socket->waitForConnected(3000))
    {
        std::cout << "* * * Połączenie z serwerem nawiązane! * * *\n";
    }

    else
    {
        std::cout << "* * * Nie udało się nawiązać połączenia z serwerem! * * *\n";
        socket->close();
        exit(EXIT_FAILURE);
    }

    selectionMenu();
}
