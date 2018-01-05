#include <iostream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <vector>
#include <pugixml.hpp>
#include <SFML/Network.hpp>

#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "salvadorgroc"
#define DATABASE "dbgame"

#ifndef DEBUG
#define PUGIXML_HEADER_ONLY
#endif

#include <pugixml.hpp>

using namespace std;

sf::TcpListener dispatcher;
sf::TcpSocket incoming;

pugi::xml_document doc;

int
iIdJugador,
iIdRaza;

std::string
sUserNick,
sUserPass,
sUserPassRepeat,
sUserRaces,
sCharacterName,
sPlayerPosition,
sPlayerSelection,
sCharacterSelected;

std::vector<string> sNameRaces;

bool
bVerified = false,
bPlayerCreated = false,
bRepeatPassword = false,
bRaceCreated = false,
bCharacterCreated = false;

int main()
{


    //Iniciamos el listener para aceptar conexiones
    sf::Socket::Status status = dispatcher.listen(5000);

    if(status != sf::Socket::Done)
    {

        std::cout << "No se ha podido vincular el puerto" << std::endl;

    }
    else
    {

        std::cout << "Puerto vinculado" << std::endl;

    }


    while(true)
    {
        if(dispatcher.accept(incoming) == sf::Socket::Done)
        {

            std::cout << "Recibo mierda" << std::endl;

        }
    }

    return 0;
}
