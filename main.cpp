#include <iostream>
#include <unistd.h>
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

pid_t
pNuevoUsuario;


char
cBufferSocket[100];

sf::TcpListener dispatcher;
sf::TcpSocket socket;
sf::Socket::Status status;

pugi::xml_document doc;

int
iIdJugador,
iIdRaza;

size_t
received;


std::string
sUserNick,
sUserPass,
sUserPassRepeat,
sUserRaces,
sCharacterName,
sPlayerPosition,
sPlayerSelection,
sCharacterSelected,
sDataReceived;

std::vector<string> sNameRaces;

bool
bVerified = false,
bPlayerCreated = false,
bRepeatPassword = false,
bRaceCreated = false,
bCharacterCreated = false,
bCloseServer = false;



void gameloop()
{

    try
    {
        //Iniciamos conexion con la base de datos
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);
        sql::Statement* stmt = con->createStatement();

        //El proceso esperara la recepcion del usuario
        sf::Socket::Status status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
        sUserNick = cBufferSocket;

        //Comprobamos si existe el nick del jugador
        sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");

        if(res->next() && res->getInt(1) == 1)
        {

            //Existe usuario
            cBufferSocket[0] = '0';
            cBufferSocket[1] = '\0';

            //Informamos que el usuario es correcto
            status = socket.send(cBufferSocket, sizeof(cBufferSocket) + 1);

            //Esperamos respuesta de la contraseña
            status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
            sUserPass = cBufferSocket;

            //Liberamos resultset anterior
            delete(res);

            //Comprobamos si existe usuario con dichos datos
            sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");

            if(res->next() && res->getInt(1) == 1) //Existe usuario con contraseña
            {

            std::cout << "Existe usuario con contraseña!" << std::endl;
            while(true);
            }




        }

        while(true)
        {


        }
    }
    catch(sql::SQLException &e)
    {

        std::cout << "Ha petao con el error " << e.getErrorCode() << std::endl;

    }
}

int main()
{

    //Iniciamos el listener para aceptar conexiones
    status = dispatcher.listen(5000);


    //Haremos que el socket y el dispatcher no bloquen el hilo principal
    dispatcher.setBlocking(false);
    socket.setBlocking(false);


    if(status != sf::Socket::Done)
    {

        std::cout << "No se ha podido vincular el puerto" << std::endl;
        return 0;

    }
    else
    {

        std::cout << "Puerto vinculado" << std::endl;

    }

    //El servidor permanecera activo hasta que lo queramos apagar
    while(!bCloseServer)
    {

        //Comprobamos si hay conexiones.
        if(dispatcher.accept(socket) == sf::Socket::Done)
        {

            //Creamos un proceso para cada conexion.

            pNuevoUsuario = fork();

            if(pNuevoUsuario == 0)
            {
                //Para el formulario haremos que el socket se bloque esperando respuesta del cliente
                socket.setBlocking(true);
                gameloop();

            }
        }
    }



    return 0;
}
