#include <iostream>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <vector>
#include <pugixml.hpp>

#define HOST "tcp://127.0.0.1:3306"
#define USER "root"
#define PASSWORD "salvadorgroc"
#define DATABASE "dbgame"

#ifndef DEBUG
#define PUGIXML_HEADER_ONLY
#endif

#include <pugixml.hpp>

using namespace std;

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
    try
    {

        //Conectamos a base de datos.
        sql::Driver* driver = get_driver_instance();
        sql::Connection* con = driver->connect(HOST, USER, PASSWORD);
        con->setSchema(DATABASE);
        sql::Statement* stmt = con->createStatement();

        while(!bVerified)
        {

            //Preguntamos nombre de usuario
            std::cout << "Inserte nombre de usuario: " << std::endl;
            std::cin >> sUserNick;

            //Comprobamos si existe el nick del jugador
            sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");


            if(res->next() && res->getInt(1) == 1) //Existe usuario
            {
                //Pedimos contraseña
                std::cout << "\nIntroduzca contraseña" << std::endl;
                std::cin >> sUserPass;

                //Liberamos resultset anterior
                delete(res);

                //Comprobamos si existe usuario con dichos datos
                sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");


                if(res->next() && res->getInt(1) == 1) //Existe usuario con contraseña
                {
                    //Liberamos resultset anterior
                    delete(res);

                    //Obtenemos el id del jugador
                    res = stmt->executeQuery("SELECT JugadorID FROM Jugadores WHERE Nombre = '" + sUserNick + "' AND Pass = '" + sUserPass + "'");

                    while(res->next())
                    {

                        iIdJugador = res->getInt("JugadorID");
                    }

                    //Liberamos resultset anterior
                    delete(res);

                    //Listamos los personajes del jugador
                    system("clear");
                    std::cout << "Selecciones personaje. \n" << std::endl;
                    res = stmt->executeQuery("SELECT Personajes.Nombre AS pNombre, Razas.Nombre AS rNombre FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.IDJugador = "+std::to_string(iIdJugador)+"");

                    std::cout<<"Nombre     |      Raza\n"<<std::endl;
                    while(res->next())
                    {
                        std::cout<<res->getString("pNombre")<<"    |     "<<res->getString("rNombre")<<std::endl;
                    }

                    //Almacenamos id del personaje seleccionado
                    std::cin >> sCharacterSelected;


                    //Liberamos resultset anterior
                    delete(res);

                    //Se han validado los datos y puede iniciar el juego.
                    bVerified = true;
                }
                else // No existe usuario con contraseña
                {
                    std::cout << "\nNo se ha encontrado ningun usuario con esa contraseña.\n" << std::endl;
                }


            }
            else //No existe usuario
            {
                std::cout << "\nNo existe dicho usuario. Se va a proceder a crear un nuevo usuario.\n" << "\nInserte nombre de usuario" <<std::endl;
                while(!bPlayerCreated)//Mientras no este el jugador creado repetiremos
                {
                    //Pedimos al usuario que indique un nuevo usuario
                    std::cin >> sUserNick;

                    //Liberamos resultset anterior
                    delete(res);

                    //Comprobamos si el nick esta libre
                    sql::ResultSet* res = stmt->executeQuery("SELECT count(*) FROM Jugadores WHERE Nombre = '" + sUserNick + "'");
                    if(res->next() && res->getInt(1) == 1) //Existe usuario
                    {
                        std::cout << "Usuario ya en uso, inserte otro nombre de usuario." << std::endl;
                    }
                    else  //No existe usuario
                    {
                        std::cout << "\nUsuario disponible, inserte contraseña.\n";
                        while(!bRepeatPassword)//Repetimos hasta tener una contraseña valida
                        {
                            //Pedimos al usuario que escriba y repita la contraseña
                            std::cin >> sUserPass;
                            std::cout << "\nRepita contraseña.\n";
                            std::cin >> sUserPassRepeat;

                            if(sUserPass == sUserPassRepeat)//Las contraseñas coinciden
                            {

                                stmt->execute("INSERT INTO Jugadores (Nombre, Pass) VALUES ('"+ sUserNick +"', '"+ sUserPass +"')");
                                bRepeatPassword = true;
                                bPlayerCreated = true;
                            }
                            else //Las contraseñas no coinciden
                            {
                                std::cout << "\nLas contraseñas no coinciden, introduzca de nuevo la contraseña.\n";

                            }
                        }

                        system("clear");

                        //Liberamos resultset anterior
                        delete(res);

                        //Listamos las razas
                        res = stmt->executeQuery("SELECT Nombre, Descripcion FROM Razas");
                        std::cout<<"Nombre     |      Descripcion\n"<<std::endl;
                        while(res->next())
                        {
                            sNameRaces.push_back(res->getString("Nombre"));
                            std::cout<<res->getString("Nombre")<<"      |      "<<res->getString("Descripcion")<<std::endl;
                        }
                        std::cout << "\nSelecciona una raza.\n";

                        //Preguntaremos razas hasta que el usuario introduzca una disponible
                        while(!bRaceCreated)
                        {
                            std::cin >> sUserRaces;
                            for(int i = 0; i < sNameRaces.size(); i++)
                            {
                                if(sUserRaces == sNameRaces[i])
                                {
                                    bRaceCreated = true;

                                    break;
                                }
                                else if(i == 2)
                                {

                                    std::cout << "\nRaza no encontrada, seleccione una raza de la lista." << std::endl;

                                }
                            }
                        }

                        //Creacion del nombre del personaje
                        std::cout << "\nInserte nombre del personaje." << std::endl;
                        while(!bCharacterCreated)
                        {
                            //Pedimos nombre de personaje al usuario y comprobamos su disponibilidad
                            std::cin >> sCharacterName;

                            //Liberamos resultset anterior
                            delete(res);

                            res = stmt->executeQuery("SELECT count(*) FROM Personajes, Jugadores, Razas WHERE Personajes.IDJugador = Jugadores.JugadorID AND Personajes.IDRaza = Razas.RazaID AND Personajes.Nombre = '"+sCharacterName+"'");
                            if(res->next() && res->getInt(1) == 1) //Existe personaje
                            {
                                std::cout << "\nNombre ya en uso, inserte otro nombre de personaje." << std::endl;
                            }
                            else //Nombre libre
                            {
                                bCharacterCreated = true;

                                //Liberamos resultset anterior
                                delete(res);

                                //Obtenemos id del jugador
                                res = stmt->executeQuery("SELECT JugadorId FROM Jugadores WHERE Nombre = '"+sUserNick+"'");
                                while(res->next())
                                {

                                    iIdJugador = res->getInt("JugadorID");
                                }

                                //Liberamos resultset anterior
                                delete(res);

                                //Obtenemos id de la raza
                                res = stmt->executeQuery("SELECT RazaId FROM Razas WHERE Nombre = '"+sUserRaces+"'");
                                while(res->next())
                                {

                                    iIdRaza = res->getInt("RazaID");
                                }

                                //Insertamos el personaje en la base de datos
                                stmt->execute("INSERT INTO Personajes(Nombre, IDJugador, IDRaza) VALUES ('"+sCharacterName+"', "+ std::to_string(iIdJugador) +", "+ std::to_string(iIdRaza) +")");
                                system("clear");
                            }
                        }
                    }
                }
            }

            //Cerramos elementos usados para la conexion con la base de datos
            //delete(res);
            //delete(stmt);
            //delete(con);

        }

        //Iniciamos el juego
        system("clear");

        std::cout << "Empieza el juego.\n\n";

        //Cargamos archivo xml
        pugi::xml_parse_result result = doc.load_file("Mazmorra.xml");
        pugi::xml_node currentNode = doc.child("mazmorra");

        //Nos desplazamos hasta la sala inicial
        currentNode = currentNode.child("habitacion");

        //Hasta que el usuario no escriba quit no pararemos el juego
        while(sPlayerSelection != "quit")
        {

            //Imprimimos descripcion
            std::cout << currentNode.child_value("descripcion");
            std::cout << "Hacia donde quieres ir?\n" << std::endl;

            //Obtenemos las direcciones
            for(pugi::xml_node direccion = currentNode.child("conexiones"); direccion; direccion = direccion.next_sibling("conexiones"))
            {

                if(direccion.child_value("Norte") != "")std::cout << "- Norte" << std::endl;
                if(direccion.child_value("Sur") != "")std::cout << "- Sur" << std::endl;
                if(direccion.child_value("Este") != "")std::cout << "- Este" << std::endl;
                if(direccion.child_value("Oeste") != "")std::cout << "- Oeste" << std::endl;
            }

            //Almacenamos la seleccion del usuario.
            std::cout << std::endl;
            std::cin >> sPlayerSelection;


            //Si la opcion es valida obtenemos la nueva sala seleccionada por el usuario y repetimos proceso
            if(doc.child("mazmorra").find_child_by_attribute("habitacion", "id", currentNode.child("conexiones").child_value(sPlayerSelection.c_str())) != NULL)
            {
                currentNode = doc.child("mazmorra").find_child_by_attribute("habitacion", "id", currentNode.child("conexiones").child_value(sPlayerSelection.c_str()));
            }

            system("clear");
        }
    }

    catch(sql::SQLException &e)
    {
        std::cout << "Se produce el error " << e.getErrorCode()<<std::endl;
    }
    return 0;
}
