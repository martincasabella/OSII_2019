#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/sysinfo.h> //para info de sistema
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <termios.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <string.h>
#include <sys/time.h>

#define MAX_BUFF_SIZE 1024
#define SCAN_CODE_SV  "klk6mv"
#define SCAN_CODE_CL  "jlk4m6"
#define UPD_CODE_SV "llfffv"
#define UPD_CODE_CL "jnfff6mv"
#define TEL_CODE_CL "t9t934"
#define TEL_CODE_SV "telgb34"
#define EXIT_CODE "ex94.f4r.5"
#define OPTIMALBUFFERTCP 65536
#define FIRMWARE_FILENAME "./../src/client/firmware/firmware.bin"
#define IMAGE_FILE "../src/client/img/aerial_sent_image.jpg"
#define INIT_UDP "-UDP-"
#define END_UPD_CODE "skl5nf"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KNRM  "\x1B[0m" //para volver a color normal
#define KGRN  "\x1B[32m"
#define KRED  "\x1B[31m" //para imprimir mensajes de error
#define KYLW "\x1b[33m"
#define END_CODE "K94jf55"

struct satellite_info {
    char sat_id[MAX_BUFF_SIZE];
    long int uptime;
    unsigned long power_consumption;
    char firmware_version[MAX_BUFF_SIZE];

};

int8_t send_telemetry (void);
int8_t receive_update (int32_t sockfd);
int8_t send_scan (int sockfd);
int8_t get_command (int sockfd);
/*Declaro estructura para info del satellte*/
struct satellite_info current_satellite;

int main (int argc, char *argv[])
{

  /*Copiamos a estructura, version actual del firmware*/
  printf ("%s\n\n[BOOTING SYSTEM...]\n\n", KCYN);
  /*ABrimos archivo propio del satelite, en donde se halla la version del firmware
   * De actualizarse, debera mostrar la correcta (nueva)
   * */

  FILE *firmw;
  /*Abro archivo, solo como lectura*/
  firmw = fopen (FIRMWARE_FILENAME, "r");
  /*Handler de si existe o no*/
  if (firmw == NULL)
    {
      printf ("Error has occurred. binary file could not be opened\n");
      exit (EXIT_FAILURE); //Simulamos que no pudo bootear el sistema
    }
  else
    {
      char buffer[MAX_BUFF_SIZE];
      bzero (buffer, sizeof (buffer));
      fread (buffer, 1, sizeof (buffer) - 1, firmw);
      /*Actualizo version en estructura: cuando bootee, deberia tener la nueva*
       * ya que al pisar la imagen del proceso, deberia quedar. */
      strcpy (current_satellite.firmware_version, buffer);
      /*Aseguramos cierre de file descriptor*/
      fclose (firmw);
      memset (buffer, 0, sizeof (buffer));
    }

  printf ("%s\n\nCurrent irmware version: %s\n\n", KYLW, current_satellite.firmware_version);
  printf ("%s[SATELLITE OMEGA ONLINE]\n\n", KGRN);
  printf ("%sWaiting for  establishing connection..\n\n", KCYN);


  /*AHora son sockets UNIX*/
  int sockfd, servlen;
  struct sockaddr_un serv_addr;

  if (argc < 2)
    {
      fprintf (stderr, "Uso %s archivo\n", argv[0]);
      exit (0);
    }

  /*Estructura socket UNIX*/
  memset ((char *) &serv_addr, 0, sizeof (serv_addr));
  serv_addr.sun_family = AF_UNIX;
  strcpy (serv_addr.sun_path, argv[1]);
  servlen = strlen (serv_addr.sun_path) + sizeof (serv_addr.sun_family);

  if ((sockfd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      printf ("%sAn error has ocurred while creating socket %s\n", KRED, strerror (errno));
      exit (-1);
    }

  if (connect (sockfd, (struct sockaddr *) &serv_addr, servlen) < 0)
    {
      printf ("%sAn error has ocurred while setting socket to CONNECT %s\n", KRED, strerror (errno));
      exit (-1);
    }

  /*Se produjo la conexion con la estacion terrestre*/
  printf ("%s\n\n[STATION NOW ONLINE]\n\n", KYLW);
  printf ("%s\n\n[STATUS: connected]\n\n", KGRN);



  /*Variable utilizada para poder recibir mas de un comando:se setea en 0 para no recibir
   * mas comandos unicamente en caso de la ejecucion de shutdown now",
   * lo que implica que el usuario desea salir del sistema (y por ende chau conexion).
   * */
  int8_t receive = 1;


  /*Bucle, donde una vez conectado, siempre esta a la espera de recepcion de uno de los comandos*/
  while (receive == 1)
    {
      /*Para poder ejecutar mas de un comando salvo que el usuario quiera tirar abajo la conexion*/
      int8_t command;
      /*Llamoado a funcion encargada de decodificar el comando recibido*/
      command = get_command (sockfd);

      switch (command)
        {
          /* 2  -    update firmware: satelite (cliente) recibe actualizacion de firmware
           * 3  -    start scanning: satelite (cliente) envia imagen aerial escaneada
           * 4  -    get telemetry: satelite (cliente) envia informacion telemetrica
           * 5  -    shutdown now: usuario (estacion) desea desconectarse.
           * 6  -    se lee algo del socket, pero no es ningun comando de los validos: imprime dicha recepcion
           *
           * -1 -     se trata de leer el socket y no hay nada recibido.
           * */

          case 2:
            /*SIno, pisa imagen de proceso, y reinicia*/
            if (receive_update (sockfd) == -1)
              {/*Hubo error en la funcion update_firwmare*/
                printf ("\n%s[ERROR - COULD NOT UPDATE SYSTEM]\n", KRED);
              }

          receive = 1;
          break;

          case 3:
            /*Envio escaneo al servidor (estacion*/
            if (!send_scan (sockfd))
              {
                printf ("%s\n\n[IMAGE SENT CORRECTLY]\n\n", KGRN);
              }
            else
              {
                printf ("%s\n\n[COULD NOT SEND IMAGE]\n\n", KRED);
              }

          receive = 1;
          break;

          case 4:
            /*Envio informacion del satelite (id, consumo, version de firmware, entre otras)*/
            if (!send_telemetry ())
              {
                printf ("\n%s[SYS INFO SENT CORRECTLY]\n\n", KGRN);
              }
            else
              {
                printf ("\n%s[ERROR WHILE SENDING TELEMETRY\n", KRED);
              }

          receive = 1;
          break;

          case 5:
            /*Uusario quiere desconectarse (ingreso shutdown now)*/
            printf ("\n\n%s[CHANNEL OFF]\n%sClosing sockets..\n[SATELLITE %sOFFLINE%s]\n", KYLW, KCYN, KNRM, KRED);
          /*Cortamos comunicacion*/

          close (sockfd);
          receive = 0;
          break;

          case 6:
            printf ("\n%sData received OK", KGRN);
          receive = 1;
          break;

          case -1:

            receive = 1;
          break;

          default:
            printf ("\n%sCannot interpret input. Retry\n", KRED);
          receive = 1;
          break;
        }

    }

  close (sockfd);
  printf ("%s\n\nDisconnecting..\n", KGRN);
  return 0;

}

/**
 * brief: obtiene el comando enviado a traves del socket una vez establecida la conexion.
 * Se da chance a que constantemente chequee a no ser que el usuario tire abajo la conexion mediante
 * el comando exit. Esto es asi por si algun mal tipeo o algun error durante la transferencia de comandos
 * ocurre, y no termina el programa. Del lado del servidor se implemento lo mismo
 *
 * @param :  sockfd - file descriptor del socket creado (TCP)
 * @return :    2  - si detecto update firmware
 *              3  - si detecto start scanning
 *              4 - si detecto get telemetry
 *              5 - si el usuario quiso abandonar la conexion (exit)
 *              6 - si recibio algo y no interpreta el comando
 *              -1 - Algun error o recepecion no interpretada
 *
 *
 * */
int8_t get_command (int sockfd)
{

  char buffer[MAX_BUFF_SIZE], op_buffer[MAX_BUFF_SIZE];
  long bytes_read = 0;
  char *token = NULL;


  /*Loop mientras lea alguna cantidad de bytes*/
  while ((bytes_read = recv (sockfd, buffer, sizeof (buffer) - 1, 0)) != 0)
    {

      /*Si entre y no leyo o es negativo, paso algo*/
      if (bytes_read <= 0)
        {
          perror ("An error has ocurred while reading socket..");
          return -1;
        }
      else
        {
          /*Leyo algo: agrego caracter nulo y meto comando leido al buffer*/
          buffer[bytes_read] = '\0';
          strcpy (op_buffer, buffer);
          token = strtok (op_buffer, "\n");
          if (token != NULL)
            {
              /*Comparo segun los codigos, que comando recibi*/
              if (!(strcmp (token, UPD_CODE_SV)))
                { //enviar archivo firmware.bin
                  write (sockfd, UPD_CODE_CL, sizeof (UPD_CODE_CL));
                  return 2;

                }
              else if (!(strcmp (token, SCAN_CODE_SV)))
                { //recibir imagen
                  write (sockfd, SCAN_CODE_CL, sizeof (SCAN_CODE_CL));
                  return 3;
                }
              else if (!(strcmp (token, TEL_CODE_SV)))
                { //abrir socket UDP para escuchar
                  write (sockfd, TEL_CODE_CL, sizeof (TEL_CODE_CL));
                  return 4;
                }
              else if (!(strcmp (token, EXIT_CODE)))
                {
                  printf ("%s[Closing connection..]\n\n", KMAG);
                  return 5;
                }
              else
                {
                  printf ("%s Received package: %s", KMAG, buffer);
                  memset (buffer, 0, sizeof (buffer));
                  return 6;
                }
            }
          else
            {
              return -1;

            }
        }
    } //end while
  /*Algo anduvo mal, leyo o no se interpreto*/
  return -1;

}//end function






/**
 * brief: envia informacion acerca del satelite, almacenada en la estructura creada.
 * La misma consta de un id del satelite, su consumo, uptime, y la version de firmware, que
 * en caso de haberse actualizado, retorna la version mas reciente.
 *
 * return : -1   - en caso de algun error
 *          0    - telemetria exitosamente enviada
 *
 * */

int8_t send_telemetry (void)
{

  /*Este buffer es para mandar toda la info*/
  char sys_info[3000];
  char buffer[MAX_BUFF_SIZE];
  memset (buffer, 0, MAX_BUFF_SIZE);
  /*sysinfo() returns certain statistics on memory and swap usage, as
    well as the load average.*/
  struct sysinfo s_info;
  int errnumber;
  /*
   * struct sysinfo
   * {
   *        long uptime;                    Seconds since boot
   *        unsigned long loads[3];         1, 5, and 15 minute load averages
   *        unsigned long totalram;         Total usable main memory size
   *        unsigned long freeram;          Available memory size
   *        unsigned long sharedram;        Memory used by buffers
   *        unsigned long bufferram;        Total swap space size
   *        unsigned long totalswap;        Swap space still available
   *        unsigned long freeswap;         Number of current processes
   *        unsigned short procs;           Pads structure to 64 bytes
   *        char _f[22];
   * }
   *
   *
   * */

  if ((errnumber = sysinfo (&s_info)) != 0)
    {
      printf ("%sError while obtaining system info, [N_ERR: %d]%s\n", KRED, errnumber, KNRM);
    }

  /*With datagram sockets, specifically unix domain datagram sockets, the client has to bind() to its own endpoint,
 * then connect() to the server's endpoint*/

  /*Version del firmware*/
  strcpy (current_satellite.sat_id, "[OMEGA]");
  current_satellite.uptime = s_info.uptime;
  current_satellite.power_consumption = s_info.totalram - s_info.freeram;
  sprintf (sys_info, "%s%s[%ld]%s%s%s[%lu]", current_satellite.sat_id, "-", current_satellite.uptime, "-", current_satellite.firmware_version, "-", current_satellite.power_consumption);

  /*Socket UDP structure*/
  struct sockaddr_un client_addr;

  char *client_filename = "./unix_udp";

  int sockfd, sizeofdest;
  if ((sockfd = socket (AF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
      int err;
      err = errno;
      printf ("%sCould not open UDP socket.   errno: %s\n", KRED, strerror (err));
      return -1;
    }


  memset (&client_addr, 0, sizeof (client_addr));
  client_addr.sun_family = AF_UNIX;
  strncpy (client_addr.sun_path, client_filename, sizeof (client_addr.sun_path));

  unlink(client_filename);

  /*BIND A CLIENTE, CONNECT A SV!*/
  if ((bind (sockfd, (struct sockaddr *) &client_addr, sizeof client_addr)) < 0)
    {
      perror ("ERROR en bind()");
      exit (1);
    }
  sizeofdest = sizeof (struct sockaddr);

  printf("%s\n[SOCKET UPD ONLINE]\n", KGRN);

  printf("Esperando que el servidor abra el socket\n");

  //Esperar recepcion de estado de listo del servidor
  while (strcmp(buffer, INIT_UDP) != 0) {
      memset(buffer, 0, MAX_BUFF_SIZE);
      if (recvfrom(sockfd, buffer, MAX_BUFF_SIZE, 0, (struct sockaddr *) &client_addr,
                   (socklen_t *) &sizeofdest) < 0) {
          perror("ERROR en lectura del socket UDP");
          exit(0);
        }
    }
  printf("Lo abrio");


  //Proceder a enviar telemetria
  if (sendto(sockfd, sys_info, strlen(sys_info), 0,
             (struct sockaddr *) &client_addr, (socklen_t) sizeofdest) < 0) {
      perror("ERROR escribiendo en el socket UDP");
      exit(1);
    }

  char * finish = END_CODE;
  if (sendto(sockfd, finish, strlen(finish), 0,
             (struct sockaddr *) &client_addr, (socklen_t) sizeofdest) < 0) {
      perror("ERROR escribiendo en el socket UDP");
      exit(1);
    }
  memset(buffer, 0, MAX_BUFF_SIZE);
  printf ("DEBUG: telemetria enviada\n");
  shutdown(sockfd, 2);

  /*Destruyo socket UDP*/
  close (sockfd);
  return 0;
}

/**
 * brief: envia imagen situada en el path definido. La imagen debe existir. *
 * @param: sockfd   file descriptor del socket TCP creado
 *
 * @return:   0 - Envio exitoso
 *           -1 - SI hubo algun emergente*
 *
 *
 * */
int8_t send_scan (int sockfd)
{

  int imgfd;
  struct stat stat_buffer;
  if ((imgfd = open (IMAGE_FILE, O_RDONLY)) < 0)
    {
      printf ("\n\n%s[IMAGE FILE DOES NOT EXIST]\n", KRED);
      return -1;
    }

  ssize_t count;
  char send_buffer[OPTIMALBUFFERTCP];
  fstat (imgfd, &stat_buffer);
  off_t fsize = stat_buffer.st_size;
  printf ("\nFilesize: %li\n", fsize);
  uint32_t bytes = htonl ((uint32_t) fsize);
  char *sendbytes = (char *) &bytes;

  if (send (sockfd, sendbytes, sizeof (bytes), 0) < 0)
    {
      perror ("\n\n[ERROR WHILE SENDING IMAGE]\n");
      return -1;
    }

  while ((count = read (imgfd, send_buffer, OPTIMALBUFFERTCP)) > 0)
    {
      if (send (sockfd, send_buffer, (size_t) count, 0) < 0)
        {
          perror ("\n\n[ERROR WHILE SENDING IMAGE]\n");
          return -1;
        }
      memset (send_buffer, 0, MAX_BUFF_SIZE);
    }
  close (imgfd);
  return 0;
}

/**
 * brief: recibe el archivo binario enviado, y lo carga a su carpeta firmware, en donde se haya
 * un archivo con la version del firmware actual. EL mismo debe ser abierto y modificado,
 * Luego, se reinicia el sistema, y deberia tener la version de firmware mas reciente.
 *
 * @param: sockfd  : descriptor del socket
 *
 * @return:    -1 -  algun tipo de error
 *
 * exit(0) si se recibio y cargo bien, pisando la imagen del proceso y booteando (execv[])
 *
 *
**/

int8_t receive_update (int32_t sockfd)
{

  int recv_size = 0, size = 0, packet_index = 1, stat;
  char binary_array[MAX_BUFF_SIZE];
  FILE *binary;

  /*Tamano del binario*/
  stat = read (sockfd, &size, sizeof (size));
  printf ("\nPacket received.\n");
  printf ("Packet size: %i\n", stat);
  printf ("binary size: %i\n", size);
  printf (" \n");
  fflush (stdout);
  write (sockfd, &size, sizeof (size));

  binary = fopen (FIRMWARE_FILENAME, "wb");

  if (binary == NULL)
    {
      printf ("\n\n%s[COULD NOT OPEN BINARY FILE - Check path]\n", KRED);
      return -1;
    }

  while (recv_size < size)
    {
      int read_size, write_size;
      memset (binary_array, 0, sizeof (binary_array));
      read_size = read (sockfd, binary_array, MAX_BUFF_SIZE);
      printf ("Packet: %i\n", packet_index);

      //Write the currently read data into our binary file
      write_size = fwrite (binary_array, 1, read_size, binary);
      if (read_size != write_size)
        {
          printf ("\nError in sizes\n");
          return -1;
        }

      recv_size += read_size;
      packet_index++;
      printf ("\nTotal received binary size: %i\n", recv_size);
      printf (" \n");
    }

  fclose (binary);
  printf ("%s\nFirmware received correctly\n\n", KGRN);
  /*Aviso que recibi bien el firmware*/
  write (sockfd, END_UPD_CODE, sizeof (END_UPD_CODE));
  printf ("%s\nRe-run program for applying new update\n\n", KYLW);
  exit (0);

}

