#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
//#include <sys/stat.h>
//#include <signal.h>
#include <time.h>

/*Cambiar en el cliente en caso de querer cambiarlo*/
#define TCP_PORT 3210
#define UDP_CLPORT 3210

#define MAX_BUFF_SIZE 1024
#define OPTIMALBUFFERTCP 65536
#define INIT_UDP "-UDP-"
#define SCAN_CODE_SV  "klk6mv"
#define SCAN_CODE_CL  "jlk4m6"
#define UPD_CODE_SV "llfffv"
#define UPD_CODE_CL "jnfff6mv"
#define TEL_CODE_CL "t9t934"
#define TEL_CODE_SV "telgb34"
#define EXIT_CODE "ex94.f4r.5"
#define END_CODE "K94jf55"
#define END_UPD_CODE "skl5nf"

/*Para tirar colores por consola*/
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KYLW "\x1b[33m"
#define KNRM  "\x1B[0m" //para volver a color normal
#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m"
#define KBLUE  "\x1b[34m"




#define MAX_CLIENTS 1
#define N_TRIES   3

/*Paths de archivos: modificar de ser necesario*/
#define IMAGE_FILE "../src/server/img/earth_aerial_img_recv.jpg"
#define FIRMWARE_FILE "../src/server/firmware/firmware.bin"
#define DBASE_PATH "../src/server/database/systemDB.txt"

/*Prototipo de funciones*/
int8_t user_login (void);
int8_t parse_command (void);
int32_t start_server (uint16_t port);
int8_t update_firmware (int sockfd);
int8_t get_telemetry (char *ipaddr);
int8_t start_scanning (int sockfd);

struct user_info {
    char usrname[20];
    char host[MAX_BUFF_SIZE];

} usr = {{0},
         {0}};

int main (int argc, char *argv[])
{


  char buffer[MAX_BUFF_SIZE];


  /*Obtenemos logg del usuario*/
  int8_t usr_status = user_login ();

  /*
   * -1: exceso de intentos incorrectos
   * -2: error en DB
   * 0: Loggeo correcto
   *
   *
   * */

  if (usr_status == -1)
    {
      printf ("%s[USER NOT AUTHORIZED]...\n\n EXITING SHELL", KRED);
      exit (EXIT_SUCCESS);
    }
  else if (usr_status == -2)
    {
      printf ("%s[DATA BASE NOT FOUND]..\n\n EXITING SHELL%s", KRED, KNRM);
      exit (EXIT_SUCCESS);

    }

    /*Uusario autenticado?*/
  else if (usr_status == 0)
    {

      int32_t len_cliaddr;

      /*Para saber si el usuario quiere apagar el sistema o interactuar*/
      int8_t usr_exit = 0;
      int8_t sat_update = 0;
      /*Mientras el usuario no haya querido salir (i.e quiere interactuar con satelite) leo su entrada*
       * Ademas si actualizo el firmware, vuelve a levantar el socket para seguir interactuando.
       */
      while ((!usr_exit && !sat_update) || (!usr_exit && sat_update))

        {
          int newsockfd;
          struct sockaddr_in client;
          int sockfd = start_server (TCP_PORT);
          printf ("%sEstablishing connection.. please wait%s\n", KYLW, KNRM);

          /*
           * accept() devuelve un file descriptor positivo si fue exitoso,-1 en caso contrario.
           *
           * El file descriptor devuelto por accept() se usará para las comunicaciones siguientes con ese cliente
           * hasta el cierrede conexión.
           *
           * El servidor sigue listo para recibir y gestionar nuevas conexiones, hasta que se supere el valor
           * MAX_CLIENTS.
           *
           * */

          /*Se bloquea el proceso hasta conexion emergente:
           * The accept function waits if there are no connections pending, unless the socket
           * socket has nonblocking mode set.*/
          len_cliaddr = (int32_t) sizeof (struct sockaddr_in);
          newsockfd = accept (sockfd, (struct sockaddr *) &client, (socklen_t *) &len_cliaddr);
          if (newsockfd == -1)
            {
              perror ("An error has ocurred during ACCEPT\n");
              exit (EXIT_FAILURE);
            }

          close (sockfd);

          /*Proceso hijo: iniciamos parseo de comandos y ejecucion*/
          printf ("\n\n%s[STATUS: ONLINE]%s\n", KGRN, KNRM);

          int8_t interact = 1; //Para ver si el usuario quiere seguir ejecutando comandos validos
          /*Espero comandos*/

          /*
           *  4   exit
           *  3   get telemetry
           *  2   start scanning
           *  1   update firmware
           *
          */
          while (interact)
            {


              int8_t usr_input = parse_command ();

              switch (usr_input)
                {

                  case 4:
                    /*Si salio del while,  ingreso exit, por lo que cerramos y chau shell*/
                    write (newsockfd, EXIT_CODE, sizeof (EXIT_CODE));
                  /*Apaga sistema, por lo que salimos del bucle de espera de comandos
                   * y cerramos estructuras posteriormente*/

                  sat_update = 0;
                  interact = 0;
                  break;

                  case 3:

                    /*Get telemetria*/
                    printf ("%s\n\n[Querying satellite..]\n", KYLW);
                  /*Le pido la telemetria al satelite enviando codigo conocido (despues se creara
                   * socket UDP para el envio)*/
                  write (newsockfd, TEL_CODE_SV, sizeof (TEL_CODE_SV));
                  /*Leo si lo interpreto*/
                  read (newsockfd, buffer, sizeof (buffer));
                  if (!strcmp (buffer, TEL_CODE_CL))
                    {
                      printf ("%s\n[Telemetry requested received]\n\n", KYLW);
                      get_telemetry (inet_ntoa (client.sin_addr));

                    }
                  else
                    { /*No ejecuto nada, y aviso que hubo error, pero sigo a la espera de mas comandos*/
                      printf ("%s\nError get telemetry.\n", KRED);
                    }

                  sat_update = 0;
                  interact = 1;
                  break;

                  case 2:

                    printf ("%s\n\n[Querying SCANNING..]\n", KBLUE);
                  /*Desde la funcion se interactua con el socket para avisar comando ingresado*/
                  if (!start_scanning (newsockfd))
                    {
                      printf ("%s[SCANNED IMAGE RECEIVED CORRECTLY]\n", KGRN);
                      printf("%sYou can find it here: %s\n\n", KYLW, IMAGE_FILE);
                    }
                  else
                    {
                      printf ("%s[DATA LOST]\n\n", KRED);
                    }
                  /*Se ejecuto bien por lo que tomo entrada, y vuelvo a darle la consola al usuario*/

                  sat_update = 0;
                  interact = 1;
                  break;

                  case 1:
                    printf ("%s\n[UPDATE REQUESTED]\n", KMAG);
                  /*Ejecuto funcion y recibo errores*/
                  if (!update_firmware (newsockfd))
                    {
                      printf ("%s\n[UPDATE DONE]\n\n", KGRN);
                      sat_update = 1;
                    }
                  else
                    {
                      printf ("%s[UPDATE ABORTED]\n\n", KRED);
                    }

                  interact = 0;
                  break;

                  default:
                    printf ("%s\n\nFollowing options are available:", KBLUE);
                  printf ("\n%s-update firmware\n%s-start scanning\n%s-get telemetry\n%s-shutdown now%s\n\n", KCYN, KYLW, KMAG, KGRN, KRED);

                  sat_update = 0;
                  interact = 1;
                  break;

                } //end switch
            } //end interact

          if (sat_update)
            {
              usr_exit = 0;
              close (newsockfd);
              printf ("\n\n%s[SATTELLITE UPDATED]\n", KYLW);
              printf ("\n\n%s[Restarting connection with satellite..]%s\n", KCYN, KNRM);

            }
          else
            {
              /*Destruyo socket*/
              close (newsockfd);
              printf ("\n\n%s[SYSTEM OFF]\n\n", KRED);
              usr_exit = 1;

            }









//        } //else pid
//          else
//            {
//              close(newsockfd);
//
//              if ((pid = wait(&status)) == -1) perror("wait error");
//              else {
//                  if (WIFSIGNALED(status) != 0)
//                    printf("Child process ended because of signal %d\n",WTERMSIG(status));
//                  else if (WIFEXITED(status) != 0)
//                    printf("Child process ended normally; status = %d\nn", WEXITSTATUS(status));
//                  else
//                    printf("\n%sChild process did not end normally\nn", KGRN);
//                }
//                printf("Parent process ended.n");
//
//              /*Para matar a ambos procesos (hijo y padre)*/
//              exit(EXIT_SUCCESS);
//            }




        } //end while

      exit (EXIT_SUCCESS);
    } //end if/else status

}

/**
 * brief:  levanta el socket TCP del servidor. Lo configura para que reuse la direccion
 * local (SO_REUSEADD)
 *
 * @param:  puerto para ligar al socket
 * @return: file descriptor del socket creado
 *
 * */
int32_t start_server (uint16_t port)
{

  int sockfd;
  struct sockaddr_in serv_addr;
  memset (&serv_addr, 0, sizeof (serv_addr));

  serv_addr.sin_family = AF_INET; //IPV 4
  /*You can use this constant to stand for “any incoming address”
   * when binding to an address*  [GNU Libc]*/
  serv_addr.sin_addr.s_addr = INADDR_ANY; //acepta entonces cualquier conexion entrante
  serv_addr.sin_port = htons (port);




  /* The SOCK_STREAM style is like a pipe. It operates over a connection with a particular
   * remote socket and transmits data reliably as a stream of bytes.
   *
   *  [GNU Libc]
   * */

  /*Creacion socket TCP*/
  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      printf ("%sAn error has ocurred while creating socket TCP [Server] %s\n", KRED, strerror (errno));
      exit (-1);
    }


  /*Pedimos autenticacion al usuario*: esta funcion devuelve 0 si se autentico correctamente,
   * y en caso contrario: -1 si se agotaron los intentos de loggeo, -2, error en la DB
   * */

  /*Seteo de parametros de CFG del Socket*/
  const int true_val = 1; //es para asegurar que si o si reciba un NON-ZERO VALUE
  int socksize;
  unsigned int restr = sizeof (socksize);

  /*
   * The close call only marks the TCP socket closed. It is not usable by process anymore.
   * But kernel may still hold some resources for a period (TIME_WAIT, 2MLS etc stuff).
   *
   * Setting of SO_REUSEADDR should remove binding problems. So be sure that value of true is really non-zero
   * when calling setsockopt (overflow bug may overwrite it):
   *
   * true = 1;
   * setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int))
  * */

  /* SO_REUSEADDR:   Reports whether the rules used in validating addresses supplied to bind()
  * should allow reuse of local addresses, if this is supported by the protocol.
  * This option stores an int value. This is a boolean option. *
   *
   *
  *  SOL_SOCKET is the socket layer itself. It is used for options that are protocol independent.
  *
  * */
  if ((!setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &true_val, sizeof (true_val))))
    {
      printf ("%s\n[SOCKET CFG LOADED]\n", KGRN);
    }
  else
    {
      printf ("%s\n[SOCKET CFG ERROR]\n\n", KRED);
    }

  /*Dado que no necesito ningun argumento especial, utilizo write/read (I/O)
   *
   * The send function is declared in the header file sys/socket.h.
   * If your flags argument is Zero, you can just as well use write instead of send;
   * [GNU libc]
   *
   * */

  /*Obtiene informacion del socket y la almacena em la variable apuntada (para tener el tamano
   * del buffer TCP)*/
  getsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, (void *) &socksize, &restr);
  /*SO_RCVBUF: Reports receive buffer size information. This option shall store an int value.*/
  /*int getsockopt (int socket, int level, int optname, void *optval, socklen t *optlen-ptr)*/


  /*Le asociamos la direccion al socket*/
  if (bind (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) == -1)
    {
      printf ("%s\nAn error has ocurred while binding socket [Server] %s\n", KRED, strerror (errno));
      exit (-1);
    }
  /*Gestionamos conexiones, hasta MAX_CLIENTS aceptadas*/
  if (listen (sockfd, MAX_CLIENTS) == -1)
    {
      printf ("%sAn error has ocurred while setting socket to LISTEN [Server] %s\n", KRED, strerror (errno));
      exit (-1);

    }
  else
    printf ("%s\n[SOCKET TCP STATUS : ONLINE]\n\nProcess: %d Socket up - PORT: %d\n\n", KGRN, getpid (), serv_addr.sin_port);

  return sockfd;
}

/**
 *  Funcion para el loggeo: ue pide usuario y contrasena al usuario.
 *  Chequea que el mismo exista en la DB, brindando N_TRIES posibles intentos de mal tipeo o mal ingreso de
 *  alguno de los dos campos.
 *
 *  @return  0  logeo exitoso
 *  @return -1  intentos agotados
 *  @return -2  error abriendo la DB
 * **/
int8_t user_login (void)
{

  char user[20]; //Especificar 19 en scanf (20 con el null terminator)
  char userpwd[15]; //Especificar 14 en scanf (15 con el null terminator)
  char line[32], comp_line[32];

  for (int i = 0; i < N_TRIES; i++)
    {

      /*Pido usuario*/
      printf ("\n%sEnter Username:", KNRM);
      scanf ("%19s", user);

      /*Simple hiding para el password (acatando shell de linux) */
      struct termios term, term_orig; //Estructura termios
      tcgetattr (STDIN_FILENO, &term);
      term_orig = term;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
      term.c_lflag &= ~(unsigned) ECHO;
#pragma GCC diagnostic pop
      tcsetattr (STDIN_FILENO, TCSANOW, &term);
      printf ("\nEnter your password : ");
      scanf ("%14s", userpwd);
      /* Remember to set back, or your commands won't echo!!!!!! */
      tcsetattr (STDIN_FILENO, TCSANOW, &term_orig); //Vuelta a la normalidad

      /*Formato user:pass*/
      comp_line[0] = '\0';
      strcat (comp_line, user);
      strcat (comp_line, ":");
      strcat (comp_line, userpwd);

      FILE *filep; //DDBB con ususarios


      /*Busco por linea en el archivo, la extraigo, y luego saco el /n. Esto es
       * porque mi cadena no lo incluye (incluye \0, y ademas, el final del archivo, i.e
       * la ultima linea, me jode*/
      if ((filep = fopen (DBASE_PATH, "r")) == NULL)
        {
          printf ("Error while reading DATABASE\n");
          return (-2);
        }

      while (fgets (line, sizeof (line), filep))
        {
          char *pch; //Para extraer lineas
          if ((pch = strchr (line, '\n')) != NULL) *pch = '\0';
          if (!strcmp (line, comp_line))
            {
              fclose (filep); //Cierro DB
              strcpy (usr.usrname, user); //seteo estructura con info del usuario
              gethostname (usr.host, MAX_BUFF_SIZE); //Choreamos el host tambien
              printf ("\n%sWelcome %s\n", KGRN, user); //aviso de logueo correcto
              return 0;
            }
        }
      /*NO hay match con ninguno de todos los user:pass existentes en la db*/
      printf ("%s\nWrong user or password. %d tries left\n", KRED, (N_TRIES - (i + 1)));
    }

  /*Logg mal en mas de N_TRIES intentos*/
  return -1;
}

/**
 * brief : parsea el comando ingresado por el usuario, dando posibilidad a que una vez loggueado pueda
 * insertar erroneamente mas de una vez. Solamente devuelve comandos validos
 *
 * @return   1:  si ingreso update firmware.bin
 *           2:  si ingreso start scanning
 *           3:  si ingreso get telemetry
 *           4:  si ingreso exit, lo que hace salir del programa.
 *
 * **/
int8_t parse_command ()
{

  int8_t valid_input = 0;
  char buffer[100];
  char opbuffer[100];
  char *read_input = buffer;

  printf ("\n%s[%s%s%s@%s%s%s] %s $ ", KMAG, KYLW, usr.usrname, KCYN, KWHT, usr.host, KMAG, KCYN);
  fgets (read_input, sizeof (buffer), stdin);

  //Loop de prompt
  while (!valid_input)
    {
      char *token_1, *token_2;
      /*Imprimo prompt con info allocada en estructura user_info*/

      //printf ("\n%s[%s%s%s@%s%s%s] %s $ ", KMAG, KWHT, usr.usrname, KCYN, KWHT, usr.host, KMAG, KCYN);
      /*Obtengo input*/

      while (!strcmp (read_input, "\n"))
        {
          fgets (read_input, sizeof (buffer), stdin);
          printf ("\n%s[%s%s%s@%s%s%s] %s $ ", KMAG, KWHT, usr.usrname, KCYN, KWHT, usr.host, KMAG, KCYN);

        }

      strcpy (opbuffer, read_input);
      /*Parto en espacio y obtengo comando hasta el espacio*/
      token_1 = strtok (opbuffer, " ");
      /*Obtengo el resto*/
      token_2 = strtok (NULL, "\n");

      if (token_1 != NULL)
        {
          if (token_2 != NULL)
            {
              char command[50];
              strcpy (command, token_1);
              strcat (command, token_2);

              if (!strcmp (command, "updatefirmware"))
                {
                  valid_input = 1;

                }
              else if (!strcmp (command, "startscanning"))
                {
                  valid_input = 2;
                }
              else if (!strcmp (command, "gettelemetry"))
                {
                  valid_input = 3;
                }
              else if (!strcmp (command, "shutdownnow"))
                {

                  valid_input = 4;
                }
              else
                {
                  printf ("%s\nBad input or wrong parsed command. Please insert again\n%s", KRED, KNRM);
                  valid_input = 0;
                  break;
                }

            } //end token2
          else
            {
              printf ("%s\nBad input or wrong parsed command. Please insert again\n%s", KRED, KNRM);
              valid_input = 0;
              break;
            }

        } //end token1

    }// end while

  /*Saldra del loop con una entrada valida*/
  return valid_input;

}//end function




/**
 * @brief Funcion encargada de enviar el archivo binario a traves del socket. Va handleando la comunicacion
 * con el satelite.
 *
 * @param: socketfd : file descritpro del socket creado
 *
 * @return   -1 : algun tipo de error
 *           0  : firmware actualizado correctamente
 *
 */
int8_t update_firmware (int sockfd)
{

  FILE *binary;
  int size;
  int packet_index = 1;
  char send_buffer[MAX_BUFF_SIZE], buffer[MAX_BUFF_SIZE];

  /*Aviso el comando ingresado al cliente mediante un write*/
  write (sockfd, UPD_CODE_SV, sizeof (UPD_CODE_SV));
  /*Leo si lo interpreto*/
  read (sockfd, buffer, sizeof (buffer));
  if (!strcmp (buffer, UPD_CODE_CL))
    {
      printf ("\n\n%sUpdating in process.. PLease wait\n\n", KYLW);
    }
  else
    {
      printf ("\n\n%s[ERROR - SATELLITE UNREACHABLE]\n", KRED);
      /*SAlgo porque no interpreto mi comando o no respondio mas el cliente*/
      return -1;
    }

  /*Abro archivo binario (tiene la nueva version)*/
  binary = fopen (FIRMWARE_FILE, "rb");
  if (binary == NULL)
    {
      printf ("\n\n%s [ERROR - Binary file could not be opened]", KRED);
      return -1;
    }

  /*Me paro en el final del archivo*/
  fseek (binary, 0, SEEK_END);
  /*Obtengo largo, i.e, tamano del archivo*/
  size = ftell (binary);
  /*Vuelvo al principio*/
  fseek (binary, 0, SEEK_SET);
  printf ("Firmware total size %i\n", size);
  printf ("Sending size... %i\n", size);
  /*Escribo bytes a enviar*/
  write (sockfd, &size, sizeof (size));
  read (sockfd, &buffer, sizeof (buffer));

  printf ("Now proceding to send firmware file...\n");
  while (!feof (binary))
    {
      int read_size;
      //Read from the file into our send buffer
      read_size = fread (send_buffer, 1, sizeof (send_buffer) - 1, binary);
      //Send data through our socket
      write (sockfd, send_buffer, read_size);
      printf ("Packet Number: %i\n", packet_index);
      printf ("Packet Size Sent: %i", read_size);
      printf ("\n");
      packet_index++;

      //Zero out our send buffer
      bzero (send_buffer, sizeof (send_buffer));
    }

  bzero (buffer, sizeof (buffer));
  read (sockfd, buffer, sizeof (buffer));
  if (!strcmp (buffer, END_UPD_CODE))
    {
      return 0;
    }

  return -1;

}

/**
 * @brief: recibe informacion enviada por el satelite: Id del satélite, Uptime del satélite, Versión del software
 *  Consumo de memoria y CPU.
 *
 *  Se emplea un socket UDP para recibir dicha informacion y mostrarla por consola.
 *
 * @param ipaddr direccion ip del satelite conectado proveniente de la estructura almacenada de la conexion tcp
 * @return 0 si recibio correctamente la informacion, -1  algun tipo de error
 */
int8_t get_telemetry (char *ipaddr)
{

  printf ("\n%sConnected to :  %s \n\n", KGRN, ipaddr);


  /*Estructuras para sockets*/
  int32_t sockfd, sizeofdest;
  /*Flag para indicar fin de transferencia de informacion*/
  uint8_t transfer_done = 0;
  char rd_buffer[MAX_BUFF_SIZE], op_buffer[MAX_BUFF_SIZE];
  char *token = NULL;
  struct sockaddr_in dest_addr;

  if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      int err;
      err = errno;
      printf ("%sCould not open UDP socket.   errno: %s\n", KRED, strerror (err));
      return -1;
    }
  memset (&dest_addr, 0, sizeof (dest_addr)); //Limpiamos estructura
  dest_addr.sin_family = AF_INET;  //familia de direcciones
  dest_addr.sin_port = htons (UDP_CLPORT); //numero de puerto
  inet_aton (ipaddr, &dest_addr.sin_addr);
  const int true_value = 1;
  setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &true_value, sizeof (true_value));

  /*COdigo de inicio de comunicacion UDP*/

  sizeofdest = sizeof (dest_addr);

  char *msg = INIT_UDP;


  /* Enviamos al satelite un mensaje para avisar del canal UDP abierto*/
  if (sendto (sockfd, msg, strlen (msg), 0, (struct sockaddr *) &dest_addr, (socklen_t) sizeofdest) < 0)
    {
      perror ("An error has ocurred while opening socket..");
      return -1;
    }

  while (!transfer_done)
    {

      //Limpio estructura
      memset (rd_buffer, 0, MAX_BUFF_SIZE);
      /*
       * The recv() call is normally used only on a connected socket (see connect(2)) a
       * nd is identical to recvfrom() with a NULL src_addr argument.All three routines return the
       * length of the message on successful completion
       * */

      //Leo del socket UDP
      if (recvfrom (sockfd, rd_buffer, MAX_BUFF_SIZE, 0, (struct sockaddr *) &dest_addr, (socklen_t *) &sizeofdest)
          < 0)
        {
          perror ("An error has ocurred while reading socket..");
          return -1;
        }

      //Recibi End of File?
      if (!strcmp (rd_buffer, END_CODE))
        {
          printf ("\n%sTelemetry received correctly.%s\n\n[Transference done].\n\n", KYLW,KGRN);
          //Limpio buffers
          transfer_done = 1;

        }
      else
        {
          /*El satelite ira enviando la informacion solicitada. La voy handleando y mostrando por pantalla
           * a medida que recibo a traves del socket*/
          printf ("\n%sReceiving telemetry information, please wait..\n", KNRM);
          strcpy (op_buffer, rd_buffer);   //Pongo lo que hay en buffer de recepcion en el de lectura
          /*Desde el cliente se envia telemetria en formato
           * info: valor   por eso obtengo hasta el token  : */
          token = strtok (op_buffer, "-");
          printf ("\n%sTELEMETRY RECEIVED:\n\n", KGRN);
          printf ("%s        Satellite ID: %s\n\n", KCYN, token);
          token = strtok (NULL, "-");
          printf ("%s        Uptime = %s [s]\n\n", KYLW, token);
          token = strtok (NULL, "-");
          printf ("%s        Software version: %s\n", KNRM, token);
          token = strtok (NULL, "\n");
          printf ("%s        Ocupied RAM (Total - free): %s bytes\n", KBLUE, token);
        }
    }

  memset (rd_buffer, 0, MAX_BUFF_SIZE);
  memset (op_buffer, 0, MAX_BUFF_SIZE);
  close (sockfd);
  return 0;

}

/**
 * @brief Obtiene la imagen (scan) del satelite por medio del socket TCP ya instanciado.
 * @param sockfd2 file descriptor del socket TCP abierto para la recepcion de la imagen
 * @return   0 : Transferencia exitosa
 *          -1 : algun error
 */
int8_t start_scanning (int sockfd)
{

  int img_fd;
  clock_t start, end;
  char recv_buffer[OPTIMALBUFFERTCP];
  long byte_read = 0;

  /*Time meassurments*/
  double cpu_time_used;
  start = clock ();

  /*Le envio el comando*/
  write (sockfd, SCAN_CODE_SV, sizeof (SCAN_CODE_SV));
  /*Leo si lo interpreto*/
  read (sockfd, recv_buffer, sizeof (recv_buffer));
  if (!strcmp (recv_buffer, SCAN_CODE_CL))
    {
      printf ("%s\n\n[Starting scanning...]\n\n", KYLW);
    }
  else
    {
      printf ("%s\n\n [Satellite unreachable]\n", KRED);
      return -1;
    }

  if ((img_fd = open (IMAGE_FILE, ((unsigned) O_WRONLY) | ((unsigned) O_CREAT) | ((unsigned) O_TRUNC), 0666)) < 0)
    {
      printf ("\n%sError while creating or opening file\n", KRED);
      return -1;
    }

  uint32_t bytesrecv;
  if ((byte_read = recv (sockfd, &bytesrecv, 4, 0)) != 0)
    {
      if (byte_read <= 0)
        {
          perror ("An error has ocurred while reading socket..\n");
        }
    }
  bytesrecv = ntohl (bytesrecv);
  printf ("Number of bytes to receive: %ud\n", bytesrecv);

  while (bytesrecv)
    {
      memset (recv_buffer, 0, OPTIMALBUFFERTCP);
      if ((byte_read = recv (sockfd, recv_buffer, OPTIMALBUFFERTCP, 0)) != 0)
        {
          //Si leo algun byte mal, aviso, y despues veo que sale
          if (byte_read <= 0)
            {
              perror ("An error has ocurred while reading socket..");
              continue;
            }
        }
      //Si quise escribir lo que recibi (emulando transferencia) y se rompio algo, rajo y pido devuelta.
      if ((write (img_fd, recv_buffer, (size_t) byte_read) < 0))
        {
          perror ("Could not write filte. Exiting..");
          return -1;
        }
      bytesrecv -= byte_read;
    }
  //gettimeofday (&end, NULL);
  close (img_fd);
  end = clock ();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf ("%s \n[RECEPTION FINISHED]\n\n%sTotal transference time: %f%s \n\n", KGRN, KYLW, cpu_time_used, KNRM);
  return 0;
}








