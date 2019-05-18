#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include <unitypes.h>
#include <libnet.h>
#include <omp.h>
#include <time.h>


/* Handle errors by printing an error message and exiting with a
 * non-zero status. */
#define ERRCODE 2
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); exit(ERRCODE);}

/* nombre del archivo a leer */
#define FILE_NAME "OR_ABI-L2-CMIPF-M6C02_G16_s20191011800206_e20191011809514_c20191011809591.nc"
#define FILE_NAME_TIME_PROC "../analysis/execution_time_procedural.txt"
#define FILE_NAME_TIME_PAR "../analysis/execution_time_parallel_v1.txt"
#define FILE_NAME_TIME_PAR2 "../analysis/execution_time_parallel_v2.txt"
#define FILE_NAME_PAR2  "../output/parallel2.nc"
#define FILE_NAME_BIN  "../output/org_img.bin"
#define FILE_NAME_BIN1  "../output/procedural_out.bin"
#define FILE_NAME_BIN2  "../output/parallel_out1.bin"
/* Lectura de una matriz de 21696 x 21696 */
#define IMAGE_WIDTH     21696
#define IMAGE_HEIGHT    21696
#define KERNEL_SIZE      3
#define OUTPUT_SIZE      (IMAGE_WIDTH - KERNEL_SIZE + 1) //por los bordes

/*Struct para matrices, mas elegante al tener diferentes tamanios (knl, out, in)*/
struct matrix_info {
    float *matrix;
    size_t col_size;
    size_t row_size;
};

/*Prototipo de funciones*/
int write_nc (const char *filename, float *data);
int procedural_convolution (struct matrix_info *img, struct matrix_info *kernel, struct matrix_info *out_img);
int parallel_convolution (struct matrix_info *img, struct matrix_info *kernel, struct matrix_info *out_img);
float *parallel_convolution_v2 (float *data_in, float kernel[][KERNEL_SIZE]);

int
main ()
{
  int ncid, varid, retval, fd;
  double exec_time;
  FILE *time_analysis;



  /* Uso de calloc -> para reservar memoria para arreglos*/
  float *data_in = (float *) calloc (IMAGE_WIDTH * IMAGE_HEIGHT, sizeof (float));
  float *data_out = (float *) calloc (
      (IMAGE_HEIGHT - KERNEL_SIZE + 1) * (IMAGE_HEIGHT - KERNEL_SIZE + 1), sizeof (float));
  float *data_out_parallel = (float *) calloc (
      (IMAGE_HEIGHT - KERNEL_SIZE + 1) * (IMAGE_HEIGHT - KERNEL_SIZE + 1), sizeof (float));


  /*La funcion para otra version de convolucion paralela, va a trabajar con float* para poder
   * crear bien el archivo netcdf (no andaba con las structs -> tiraba segfalut dump core)*/
  float *data_out_parallel_v2;

  /*Declaracion de los coeficientes del kernel (filtro de borde)*/
  float kernel_coefficients[KERNEL_SIZE][KERNEL_SIZE] = {
      {-1, -1, -1},
      {-1, 8,  -1},
      {-1, -1, -1}
  };


  /*Structs para manipular mas facil los tamanos*/
  struct matrix_info *input_image = &(struct matrix_info) {data_in, IMAGE_HEIGHT, IMAGE_WIDTH};
  struct matrix_info *output_image = &(struct matrix_info) {data_out, OUTPUT_SIZE, OUTPUT_SIZE};
  struct matrix_info *output_image_parallel = &(struct matrix_info) {data_out_parallel, OUTPUT_SIZE, OUTPUT_SIZE};
  struct matrix_info *kernel = &(struct matrix_info) {(float *) kernel_coefficients, KERNEL_SIZE, KERNEL_SIZE};

  printf ("Reading input image. Please wait..\n");

  if ((retval = nc_open (FILE_NAME, NC_NOWRITE, &ncid)))
  ERR(retval);

  /* Obtenemos elvarID de la variable CMI. */
  if ((retval = nc_inq_varid (ncid, "CMI", &varid)))
  ERR(retval);

  /* Leemos la matriz. */
  if ((retval = nc_get_var_float (ncid, varid, data_in)))
  ERR(retval);

  /* Se cierra el archivo y liberan los recursos*/
  if ((retval = nc_close (ncid)))
  ERR(retval);

  printf ("\n[IMAGE CORRECTLY READ]\n");

  /*Generamos binario con imagen original*/
  fd = open (FILE_NAME_BIN, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  int x = 0;
  while (x < (int) (IMAGE_WIDTH * IMAGE_WIDTH * sizeof (float)))
    {
      x += write (fd, input_image->matrix, (IMAGE_WIDTH * IMAGE_WIDTH * sizeof (float)));
    }
  close (fd);

  printf ("Proceeding to do PROCEDURAL convolution operation... \n");
  /* Procedural*/
  exec_time = omp_get_wtime ();
  procedural_convolution (input_image, kernel, output_image);
  exec_time = omp_get_wtime () - exec_time;
  printf ("Procedural convolution time: %f\n", exec_time);

  printf ("Writing execution time. Please wait..\n");
  time_analysis = fopen (FILE_NAME_TIME_PROC, "a"); //lo abro con append asi agrega con cada corrida
  fprintf (time_analysis, "%f\n", exec_time);
  fclose (time_analysis);

  printf ("\nSaving output.. Please wait\n");
  /*Guardo salida en archivo binario*/
  fd = open (FILE_NAME_BIN1, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  write (fd, output_image->matrix, (OUTPUT_SIZE * OUTPUT_SIZE * sizeof (float)));
  close (fd);
  printf ("\n[OUTPUT SAVED]\n");
  /*Libero memoria*/
  free (output_image->matrix);

  printf ("Now proceeding to do PARALLEL convolution..\n");
  /* Paralela v1*/
  exec_time = omp_get_wtime ();
  parallel_convolution (input_image, kernel, output_image_parallel);
  exec_time = omp_get_wtime () - exec_time;

  printf ("\n\nParallel v1 (collapse) convolution time: %f\n", exec_time);
  printf ("Writing execution time. Please wait..\n");
  time_analysis = fopen (FILE_NAME_TIME_PAR, "a"); //lo abro con append asi agrega con cada corrida
  fprintf (time_analysis, "%f\n", exec_time);
  fclose (time_analysis);

  printf ("\nSaving output.. Please wait\n\n");
  /*Guardo salida en archivo binario*/
  fd = open (FILE_NAME_BIN2, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  write (fd, output_image_parallel->matrix, (OUTPUT_SIZE * OUTPUT_SIZE * sizeof (float)));
  close (fd);
  /*Libero variable utilizada*/
  free (output_image_parallel->matrix);

  printf ("Now proceeding to do PARALLEL v2 convolution..\n");
  /* Paralela v2*/
  exec_time = omp_get_wtime ();
  data_out_parallel_v2 = parallel_convolution_v2 (data_in, kernel_coefficients);
  exec_time = omp_get_wtime () - exec_time;
  printf ("Parallel v2 (parallel for) convolution time: %f\n", exec_time);

  printf ("Writing execution time. Please wait..\n");
  time_analysis = fopen (FILE_NAME_TIME_PAR2, "a"); //lo abro con append asi agrega con cada corrida
  fprintf (time_analysis, "%f\n", exec_time);
  fclose (time_analysis);

  /*Ahora escribimos archivo .nc*/
  printf ("\nSaving output.. Please wait\n\n");
  if (write_nc (FILE_NAME_PAR2, data_out_parallel_v2) == 0)
    {
      printf ("\n[NC ARCHIVE SUCCESSFULLY WRITTEN]\n\n");
    }

  /*Free variables utilizada que quedaron*/
  printf ("Releasing last resources resources.. please wait\n");
  free (data_in);
  free (kernel->matrix);
  free (data_out_parallel_v2);
  printf ("\n[ALL RESOURCES RELEASED SUCCESSFULLY]\n\n");
  return 0;

}

/**
 * Convolucion procedural. Se almacena la imagen filtrada en *out_img, y luego se genera el binario correspondiente
 *
 * @param img: puntero a estructura matrix_info, que alberga la imagen leida del archivo netCDF
 * @param kernel: puntero a estructura matrix_info, que alberga los coeficientes del kernel junto con su tamano
 * @param out_img: puntero a estructura matrix_info, donde se almacenara la imagen filtrada,
 * cuya dimension es IMAGE_HEIGHT - KERNEL_SIZE + 1 (tanto para filas como para columnas)
 */
int procedural_convolution (struct matrix_info *img, struct matrix_info *kernel, struct matrix_info *out_img)
{
  for (size_t row = 0; row < out_img->col_size; row++)
    {
      for (size_t col = 0; col < out_img->row_size; col++)
        {
          for (size_t knl_row = 0; knl_row < kernel->col_size; knl_row++)
            {
              for (size_t knl_col = 0; knl_col < kernel->col_size; knl_col++)
                {
                  out_img->matrix[row * (out_img->row_size) + col] +=
                      (img->matrix[(row + knl_row) * (img->row_size) + (col + knl_col)])
                      * (kernel->matrix[(knl_row) * (kernel->row_size) + (knl_col)]);
                }
            }
        }
    }
  return 0;
}

/**
 * Convolucion en paralelo, utilizando OMP COLLAPSE para los dos ciclos externos (plancha los loops)
 *
 * @param img: puntero a estructura matrix_info, que alberga la imagen leida del archivo netCDF
 * @param kernel: puntero a estructura matrix_info, que alberga los coeficientes del kernel junto con su tamano
 * @param out_img: puntero a estructura matrix_info, donde se almacenara la imagen filtrada,
 * cuya dimension es IMAGE_HEIGHT - KERNEL_SIZE + 1 (tanto para filas como para columnas)
 */
int parallel_convolution (struct matrix_info *img, struct matrix_info *kernel, struct matrix_info *out_img)
{

#pragma omp parallel for collapse(2)

  for (size_t row = 0; row < out_img->col_size; row++)
    {
      for (size_t col = 0; col < out_img->row_size; col++)
        {
          if (col==5000){
              printf ("\nNumber of threads to use: %d\n", omp_get_level ());
          }
          for (size_t knl_row = 0; knl_row < kernel->col_size; knl_row++)
            {
              for (size_t knl_col = 0; knl_col < kernel->col_size; knl_col++)
                {
                  out_img->matrix[row * (out_img->row_size) + col] +=
                      (img->matrix[(row + knl_row) * (img->row_size) + (col + knl_col)])
                      * (kernel->matrix[(knl_row) * (kernel->row_size) + (knl_col)]);

                }
            }
        }
    }

  return 0;
}

/**
 * Convolucion en paralelo, utilizando parallel for
 *
 * @param data_in: imagen leida del archivo netcdf
 * @param kernel: kernel utilizado para filtrar imagen
 *
 * @return data_out: imagen filtrada de tamanio height*width definidos
 *
 */

float *parallel_convolution_v2 (float *data_in, float kernel[][KERNEL_SIZE])
{
  const int width = 21696;
  const int height = 21696;
  const float scale_factor = ((float) (255 * 255)) / ((float) (4000 * 4000));

  /*Allocamos memoria para variable de salida*/
  float *out_img = (float *) calloc (
      (width) * (height), sizeof (float));

  int j, i;

#pragma omp parallel for

  for (j = 1; j < width - 1; j++)
    {
      if (j==5000){
          printf ("\nNumber of threads to use: %d\n", omp_get_level ());
      }

      for (i = 1; i < width - 1; i++)
        {
          out_img[(i) + (j * width)] =
              ((data_in[(i - 1) + ((j - 1) * width)] * kernel[0][0]) + (data_in[(i) + ((j - 1) * width)] * kernel[0][1])
               + (data_in[(i + 1) + ((j - 1) * width)] * kernel[0][2])
               + (data_in[(i - 1) + ((j) * width)] * kernel[1][0])
               + (data_in[(i) + ((j) * width)] * kernel[1][1]) + (data_in[(i + 1) + ((j) * width)] * kernel[1][2])
               + (data_in[(i - 1) + ((j + 1) * width)] * kernel[2][0])
               + (data_in[(i) + ((j + 1) * width)] * kernel[2][1])
               + (data_in[(i + 1) + ((j + 1) * width)] * kernel[2][2])) * scale_factor;

        }
    }
  return out_img;
}

/**
 *Escritura de variable CMI de Archivo .nc
 * *@param [in] filename Nombre de archivo
 *@param [in] data Puntero a datos que se desean escribir en la variable
 */
int write_nc (const char *filename, float *data)
{
  int out_ncid, out_varid, retval;
  //Apertura y lectura de archivo original
  if ((retval = nc_open (filename, NC_WRITE, &out_ncid)))
  ERR(retval);

  // Obtenemos elvarID de la variable CMI.
  if ((retval = nc_inq_varid (out_ncid, "CMI", &out_varid)))
  ERR(retval);
  // Escribimos la matriz.
  if ((retval = nc_put_var_float (out_ncid, out_varid, data)))
  ERR(retval);

  if ((retval = nc_sync (out_ncid)))
  ERR(retval);

  if ((retval = nc_close (out_ncid)))
  ERR(retval);

  return 0;

}











