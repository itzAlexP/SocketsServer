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
sDataReceived,
sDataSender;


std::vector<string>
sNameRaces,
sNameCharacters;

bool
bPlayerCreated = false,
bRepeatPassword = false,
bNewUserVerified = false,
bRaceCreated = false,
bCharacterCreated = false,
bCloseServer = false,
bCharacterNameVerified = false,
bCharacterSelected = false;

void CrearPersonaje(sql::ResultSet* res, sql::Statement* stmt)
{
    std::string sDataSenderAux;
    bRaceCreated = false;
    bCharacterNameVerified = false;

    //Obtenemos las razas de la base de datos y las enviamos al jugador
    while(!bRaceCreated)
    {

        res = stmt->executeQuery("SELECT Nombre, Descripcion FROM Razas");
        while(res->next())
        {
            sNameRaces.push_back(res->getString("Nombre"));
            sDataSenderAux = sDataSenderAux + res->getString("Nombre") + "-" + res->getString("Descripcion") + "-";
            bRaceCreated = true;
        }

    }

    status = socket.send(sDataSenderAux.c_str(), sDataSenderAux.size() + 1);

    //Esperamos raza del jugador
    status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
    sUserRaces = cBufferSocket;

    //Esperamos nombre del personaje
    status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
    sCharacterName = cBufferSocket;

    //Comprobamos si el nombre esta libre
    res = stmt->executeQuery("SELECT count(*) FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.Nombre = '" + sCharacterName + "'");

    if(res->next() && res->getInt(1) == 1) //Nombre Ocupado
    {

        //Volvemos a pedir usuario
        while(!bCharacterNameVerified)
        {
            cBufferSocket[0] = '1';
            cBufferSocket[1] = '\0';

            //Informamos que el nombre ya existe
            status = socket.send(cBufferSocket, sizeof(cBufferSocket));

            //Esperamos respuesta de nuevo nombre
            status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
            sCharacterName = cBufferSocket;

            //Liberamos resultset anterior
            //delete(res);

            //Comprobamos si existe usuario con dichos datos
            res = stmt->executeQuery("SELECT count(*) FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.Nombre = '" + sCharacterName + "'");
            if(res->next() && res->getInt(1) != 1) //No existe usuario
            {
                cBufferSocket[0] = '0';
                cBufferSocket[1] = '\0';

                status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                bCharacterNameVerified = true;
            }
        }
    }

    //Obtenemos id del jugador para realizar el insert
    res = stmt->executeQuery("SELECT JugadorId FROM Jugadores WHERE Nombre = '"+sUserNick+"'");
    while(res->next())
    {

        iIdJugador = res->getInt("JugadorID");
    }


    //Obtenemos id de la raza
    res = stmt->executeQuery("SELECT RazaId FROM Razas WHERE Nombre = '"+sUserRaces+"'");
    while(res->next())
    {

        iIdRaza = res->getInt("RazaID");
    }

    //Realizamos insert del personaje
    stmt->execute("INSERT INTO Personajes(Nombre, IDJugador, IDRaza) VALUES ('"+sCharacterName+"', "+ std::to_string(iIdJugador) +", "+ std::to_string(iIdRaza) +")");



}

void gameloop()
{

    bool
    bPasswordVerified = false;

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

        if(res->next() && res->getInt(1) == 1) //Existe usuario
        {

            //Existe usuario
            cBufferSocket[0] = '0';
            cBufferSocket[1] = '\0';

            //Informamos que el usuario es correcto
            status = socket.send(cBufferSocket, sizeof(cBufferSocket));

            //Esperamos respuesta de la contraseña
            status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
            sUserPass = cBufferSocket;

            //Liberamos resultset anterior
            //delete(res);

            //Comprobamos si existe usuario con dichos datos
            sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");

            if(res->next() && res->getInt(1) == 1) //Existe usuario con contraseña
            {

                //Existe usuario
                cBufferSocket[0] = '0';
                cBufferSocket[1] = '\0';

                //Informamos que el usuario con contraseña es correcto
                status = socket.send(cBufferSocket, sizeof(cBufferSocket));


            }
            else //No existe usuario con contraseña
            {

                while(!bPasswordVerified)//Mientras no la escriba bien repetiremos
                {

                    cBufferSocket[0] = '1';
                    cBufferSocket[1] = '\0';

                    //Informamos que el usuario con contraseña es incorrecto
                    status = socket.send(cBufferSocket, sizeof(cBufferSocket));

                    //Esperamos respuesta de la contraseña
                    status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
                    sUserPass = cBufferSocket;

                    //Liberamos resultset anterior
                    //delete(res);

                    //Comprobamos si existe usuario con dichos datos
                    sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");
                    if(res->next() && res->getInt(1) == 1) //Existe usuario con contraseña
                    {
                        cBufferSocket[0] = '0';
                        cBufferSocket[1] = '\0';
                        status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                        bPasswordVerified = true;
                    }

                }

            }

        }
        else //No existe el usuario
        {
            while(!bPlayerCreated)//Mientras no este el jugador creado repetiremos
            {

                //No existe usuario
                cBufferSocket[0] = '2';
                cBufferSocket[1] = '\0';

                //Informamos que el usuario no existe
                status = socket.send(cBufferSocket, sizeof(cBufferSocket));


                //Recibimos nombre de usuario
                status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);

                sUserNick = cBufferSocket;

                //Liberamos resultset anterior
                //delete(res);

                //Comprobamos si el nick esta libre
                sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");

                if(res->next() && res->getInt(1) == 1) //Existe usuario
                {

                    //Volvemos a pedir usuario
                    while(!bNewUserVerified)
                    {
                        cBufferSocket[0] = '1';
                        cBufferSocket[1] = '\0';

                        //Informamos que el usuario ya existe
                        status = socket.send(cBufferSocket, sizeof(cBufferSocket));

                        //Esperamos respuesta de nuevo nombre de usuario
                        status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
                        sUserNick = cBufferSocket;

                        //Liberamos resultset anterior
                        //delete(res);

                        //Comprobamos si existe usuario con dichos datos
                        sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");
                        if(res->next() && res->getInt(1) != 1) //No existe usuario
                        {
                            cBufferSocket[0] = '0';
                            cBufferSocket[1] = '\0';

                            status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                            bNewUserVerified = true;
                        }
                    }
                }
                else
                {

                    //Informamos al servidor que el nombre esta libre
                    cBufferSocket[0] = '0';
                    cBufferSocket[1] = '\0';
                    status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                }

                while(!bRepeatPassword)
                {


                    status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
                    sUserPass = cBufferSocket;

                    status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);
                    sUserPassRepeat = cBufferSocket;

                    if(sUserPass == sUserPassRepeat)//Las contraseñas coinciden
                    {
                        cBufferSocket[0] = '0';
                        cBufferSocket[1] = '\0';

                        status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                        stmt->execute("INSERT INTO Jugadores (Nombre, Pass) VALUES ('"+ sUserNick +"', '"+ sUserPass +"')");
                        bRepeatPassword = true;
                        bPlayerCreated = true;
                    }
                    else //Las contraseñas no coinciden
                    {
                        cBufferSocket[0] = '1';
                        cBufferSocket[1] = '\0';

                        status = socket.send(cBufferSocket, sizeof(cBufferSocket));
                    }
                }


                CrearPersonaje(res, stmt);


            }
        }

        sDataSender.clear();

        //Obtenemos los personajes de la base de datos y los enviamos al jugador
        while(!bCharacterSelected)
        {


            //Obtenemos id del jugador
            res = stmt->executeQuery("SELECT JugadorId FROM Jugadores WHERE Nombre = '"+sUserNick+"'");
            while(res->next())
            {

                iIdJugador = res->getInt("JugadorID");
            }

            res = stmt->executeQuery("SELECT Personajes.Nombre AS pNombre, Razas.Nombre AS rNombre FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.IDJugador = "+std::to_string(iIdJugador)+"");

            while(res->next())
            {
                sNameCharacters.push_back(res->getString("pNombre"));
                sDataSender = sDataSender + res->getString("pNombre") + "-";

            }

            //Enviamos listado
            status = socket.send(sDataSender.c_str(), sDataSender.size() + 1);

            //Esperamos respuesta del nombre
            status = socket.receive(cBufferSocket, sizeof(cBufferSocket), received);


            //El jugador desea crear un nuevo personaje
            if(std::string(cBufferSocket) == "Nuevo" || std::string(cBufferSocket) == "nuevo")
            {

                CrearPersonaje(res, stmt);


            }
            else
            {

                bCharacterSelected = true;

            }
        }


        //Recibimos personaje seleccionado
        std::cout << "Power Ranger" << std::endl;
        while(true){}

    }
    catch(sql::SQLException &e)
    {

        std::cout << "Error: " << e.getErrorCode() << std::endl;

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
