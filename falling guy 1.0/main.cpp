
// robot.c: Adaptado de OpenGL Programming Guide (Neider, Davis, Woo)
// El movimiento de la camara se logro gracias a http://www.lighthouse3d.com/opengl/glut/index.php?6

#include <GL/glut.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
//-----------------------------------------
#define GRAVITY 0.098
#define ESCALA_PISO 1.0//Constantes para construir el piso, son el XxZ (tamanio del piso)
#define PASOS_X_PISO 200.0
#define PASOS_Z_PISO 200.0//---------------------------------

GLfloat angle, fAspect;

//--------------------------------------------------Atributos para la fisica -----------------
//Caida libre
static GLfloat tiempoCaidaLibre = 0.0, tiempoMovimiento = 0.0;
static bool colisionoPisoCaida;//Mientras el man no haya tocado el piso su valor es false, cuando ya lo haya tocado sera true

//Tiro parabolico
static GLfloat altura;//Altura desde que se tiro el man
//Se define dado la altura maxima que va a tomar el movimiento parabolico de cada parte del man
static GLfloat anguloAlfa;
static GLfloat tiempoParabolicoLeg = 0.0, tiempoParabolicoArtLeg = 0.0, tiempoParabolicoArm = 0.0, tiempoParabolicoArtArm = 0.0, tiempoParabolicoHead = 0.0;
static GLfloat deltaTiempoCaida, deltaTiempoParabolico;

//------------------------------------------------------------------------------------------------

//---------------------------------------------------Atributos de la camara-------------------------------

//  camX,camY,camZ: posicion de la camara
static GLfloat camX=0.0f,camY=20.0f,camZ=70.0f;

// camLX,camLY,camLZ: vector unitario que define la linea de vista de la camara
static GLfloat camLX=0.0f,camLY=0.0f,camLZ=0.0f;

/* angulo de rotacion. Nos permite rotar la camara, 
dependiendo del angulo, las componentes de la linea  de vista son diferentes*/
static GLfloat angleCamXZ=0.0, angleCamY= 0.0;

/*Este valor se cambia cuando el usuario presiona la tecla correspondiente y vuelve a ser 0.0 cuando la suelta
Controla cuando y cuanto debe girar la camara*/
static GLfloat deltaAngleCamXZ = 0.0, deltaAngleCamY = 0.0;

//Define cuanto se mueve la camara en XZ, si es != 0 quiere decir que se presiono una tecla, pero si es == 0 entonces solto la tecla
static int deltaMoveCam= 0;
//--------------------------------------------------------------------------------------------------

//---------------------------------------------------Atributos del man-------------------------------------
static int manAngleY,  manAngleYDelta; 
static GLfloat manPosY,manPosX,manPosZ, manMoveYDelta, manVelX , manVelY,manVelZ;
static GLfloat manSizeX, manSizeY, manSizeZ;

//Atributos de cada parte del cuerpo, el posN, con n x, y, z, siginifica la posicion en el plano x,y o z respectivamente
//El anguloBeta es la direccion en xz que va a tomar cada parte en el movimiento parabolico
//Valores del man que van a variar en la ejecucion del programa
static GLfloat legPosX ,legPosY ,legPosZ , legAnguloBeta, legVel, legLado;
static GLfloat artLegPosX ,artLegPosY,artLegPosZ, artLegAnguloBeta, artLegVel, artLegRadio;
static GLfloat torsoPosX,torsoPosY,torsoPosZ, torsoAnguloBeta, torsoLado;
static GLfloat armPosX,armPosY,armPosZ, armAnguloBeta, armVel, armLado;
static GLfloat artArmPosX,artArmPosY,artArmPosZ, artArmAnguloBeta, artArmVel, artArmRadio;
static GLfloat headPosX,headPosY,headPosZ, headAnguloBeta, headVel, radioCabeza, headRadio;
 
//Valores iniciales del man que no van a variar en la ejecucion del programa
static GLfloat init_legPosX ,init_legPosY ,init_legPosZ , init_legAnguloBeta;
static GLfloat init_artLegPosX ,init_artLegPosY,init_artLegPosZ, init_artLegAnguloBeta;
static GLfloat init_torsoPosX,init_torsoPosY,init_torsoPosZ, init_torsoAnguloBeta;
static GLfloat init_armPosX,init_armPosY,init_armPosZ, init_armAnguloBeta ;
static GLfloat init_artArmPosX,init_artArmPosY,init_artArmPosZ, init_artArmAnguloBeta;
static GLfloat init_headPosX,init_headPosY,init_headPosZ, init_headAnguloBeta;


//Texturas del torso
GLuint textureTorsoBack, textureTorsoFront,textureTorsoUp, textureTorsoSide;

//--------------------------------------------------------------------------------------------------

//Dislay list de todo lo que no tiene que ver con el man, que es estatico y no se mueve
static GLint piso_display_list;

//Es false si no ha comenzado a caer el man, es true si ya comenzo a caer
static bool comenzoAnimacion, mostrarOcultarControles = true;

//Metodos
void init (void);
void initTextures();
GLuint initPisoDL();
void initRandom();

GLuint getIDTexture( const char * filename );
char * cargarImagen(int width ,int height, const char * filename  );

bool manejarCamara(void);
void orientarCamara() ;
void moverCamara(int distancia) ;


void movimientoParabolico();
float getRandomVelocidad();
void setVelocidades();
void caidaLibre();
void manejarFisica();


void visualizationParams();
void dibujarPiso();
void dibujarTorsoConTextura(double d);
void dibujarMan();
void dibujarTextoControles();
void renderStrokeCharacter(float x, float y, float z, void *font,char *string);
void dibujarTextoControles();
void display();


// Inicializar los parametros de despliegue
void init (void)
{           
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    angle=45;
    
    piso_display_list = initPisoDL();
    
    manAngleY = 0, manAngleYDelta = 0; 
    manPosY = 12.0, manPosX = 0.0, manPosZ = 0.0, manMoveYDelta = 0.0;
    manVelX = 0.0, manVelY = 0.0, manVelZ = 0.0;
    manSizeX = 0.8, manSizeY = 0.8, manSizeZ = 0.8;
    
    init_legPosX = 0.0,init_legPosY = 1.0,init_legPosZ = 1.5, init_legAnguloBeta = 2*M_PI/3;//M_PI/8.0;
    init_artLegPosX = 1.8,init_artLegPosY = 1.0,init_artLegPosZ = 1.5, init_artLegAnguloBeta = init_legAnguloBeta;
    init_torsoPosX = 4.6,init_torsoPosY = 1.0,init_torsoPosZ = 0.0, init_torsoAnguloBeta = 90*M_PI/180.0;
    init_armPosX = 4.6,init_armPosY = 1.0,init_armPosZ = 3.5, init_armAnguloBeta = M_PI/8.0;//2*M_PI/3;
    init_artArmPosX = 4.6,init_artArmPosY = 1.0,init_artArmPosZ = 2.6,init_artArmAnguloBeta = init_armAnguloBeta;
    init_headPosX = 9.0,init_headPosY = 1.0,init_headPosZ = 0.0,init_headAnguloBeta = 0.0;
    
    legPosX = init_legPosX,legPosY = init_legPosY, legPosZ = init_legPosZ, legAnguloBeta = init_legAnguloBeta,  legLado = 3.0;//M_PI/8.0;
    artLegPosX = init_artLegPosX, artLegPosY = init_artLegPosY, artLegPosZ = init_artLegPosZ, artLegAnguloBeta = init_artLegAnguloBeta,  artLegRadio = 0.5;
    torsoPosX = init_torsoPosX, torsoPosY = init_torsoPosY, torsoPosZ = init_torsoPosZ, torsoAnguloBeta = init_torsoAnguloBeta, torsoLado = 5.0;
    armPosX = init_armPosX, armPosY = init_armPosY, armPosZ = init_armPosZ, armAnguloBeta = init_armAnguloBeta,  armLado = 3.0;//2*M_PI/3;
    artArmPosX = init_artArmPosX, artArmPosY = init_artArmPosY, artArmPosZ = init_artArmPosZ, artArmAnguloBeta = init_artArmAnguloBeta,  artArmRadio = 0.5;
    headPosX = init_headPosX, headPosY = init_headPosY, headPosZ =init_headPosZ, headAnguloBeta = init_headAnguloBeta, headRadio = 2.0;
    
    legVel = 0.0;
    artLegVel = 0.0;
    armVel = 0.0;
    artArmVel = 0.0;
    headVel = 0.0;  
    
    tiempoCaidaLibre = 0.0, deltaTiempoCaida = 0.01;
    //Cada parte rebota independiente de las otras, por esto cada una tiene su tiempo en el movimiento parabolico
    tiempoParabolicoLeg = 0.0, tiempoParabolicoArtLeg = 0.0, tiempoParabolicoArm = 0.0, tiempoParabolicoArtArm = 0.0, tiempoParabolicoHead = 0.0;
    deltaTiempoParabolico = deltaTiempoCaida *40 ;
    
    comenzoAnimacion= false;  
    colisionoPisoCaida = false;  
    
    initTextures();
    initRandom();
}

//Inicializa cada textura
void initTextures(){
    textureTorsoBack = getIDTexture( "Texturas/torso-back.raw" );
    textureTorsoFront = getIDTexture(  "Texturas/torso-front.raw" );
    textureTorsoUp = getIDTexture( "Texturas/torso-up.raw" );
    textureTorsoSide = getIDTexture( "Texturas/torso-side.raw" ); 
}

//Crea la displayList para dibujarPiso y devuelve el GLuint correspondiente a ella
GLuint initPisoDL() {
	GLuint floorDL;
	floorDL = glGenLists(1);
	glNewList(floorDL,GL_COMPILE);
		
		dibujarPiso();
	glEndList();
	return(floorDL);
}

//Seed el random   
void initRandom(){
    srand((unsigned)(time(0)));  
}

//Carga la imagen correspondiente al filename y devuelve este contenido en un char*
char * cargarImagen(int width ,int height, const char * filename  ){
// open texture data
  char * data;
  FILE * file = fopen( filename, "rb" );
  if ( file == NULL ) {
	  printf("File not found'n");
	  exit(1);
  }
  
  data = (char*)malloc( width * height * 3 );
  // read texture data
  fread( data, width * height * 3, 1, file );
  fclose( file );   
  return data;   
}

// Controla la creacion de la textura devuelve su GLuint 
GLuint getIDTexture( const char * filename )
{
  GLuint texture;
  //GLuint texture = textura;
  int width = 256, height = 256;
  char * data ;
  
  data = cargarImagen(width, height, filename);
  
  glGenTextures( 1, &texture );
  glBindTexture( GL_TEXTURE_2D, texture );
  
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP );
  
  
  gluBuild2DMipmaps( GL_TEXTURE_2D, 4, width, height, GL_RGB, GL_UNSIGNED_BYTE, data );
  
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );//GL_REPLACE);
  
  free( data );
  return texture;
}

// ---------------------------------------METODOS ENCARGADOS DEL MOVIMIENTO DE LA CAMARA ----------------------------------
/*Controla que metodo llamar dependiendo, si hubo algun cambio o no en los valores anteriores, osea si el delta != 0.0,
 si hubo algun cambio retorna true, de lo contrario retorna false*/
bool manejarCamara(void){
    bool cambio = false;
    // Mueve la camara ----------------------------------------
    if (deltaMoveCam)
		moverCamara(deltaMoveCam);
        cambio = true;		
	if (deltaAngleCamXZ || deltaAngleCamY) {
		angleCamXZ += deltaAngleCamXZ;
		if(angleCamY + deltaAngleCamY > -3.0 && angleCamY + deltaAngleCamY < 3.0)angleCamY += deltaAngleCamY;
		orientarCamara();
		cambio = true;
	}
    return cambio;
	//----------------------------------------------------------- 
}
//Cambia el look point (hacia donde mira) de la camara de cada componente 
void orientarCamara() {
	camLX = sin(angleCamXZ);
	camLZ = -cos(angleCamXZ);//Negativo para que el movimiento sea en direccion a la flecha presionada   
    camLY = angleCamY;//sin(angleCamY);
        glLoadIdentity();
	gluLookAt(camX, camY, camZ, 
		      camX + camLX, camY + camLY, camZ + camLZ,
			  0.0f,1.0f,0.0f);
}

//Mueve la camara a travez del espacio
void moverCamara(int distancia) {
     // Look At Point = Line Of Sight + Camera Position por que queremos mover la camara siguiendo la linea de vista
    camX = camX + distancia*camLX*(0.2);
	camZ = camZ + distancia*camLZ*(0.2);
	camY = camY + distancia*camLY*(0.2);
	glLoadIdentity();
	gluLookAt(camX, camY, camZ, 
		      camX + camLX , camY + camLY , camZ + camLZ,
			  0.0f,1.0f,0.0f);
}
//-----------------------------------------------------------------------------------------------------------------

//Implementa fisica de movimiento parabolico en 3 dimensiones para cada parte del man
void movimientoParabolico(){
   double tiempo = clock();
   double desgaste = 0.85;
   double posTemp;//Contiene la posicion en y para cada parte un momento antes de asignarla para saber si traspasa el piso o no
   
   posTemp = legVel*(sin(anguloAlfa))*tiempoParabolicoLeg - (GRAVITY*tiempoParabolicoLeg*tiempoParabolicoLeg)/2 + init_legPosY ; 
   if(posTemp - legLado/4 >= 0.0 ){  //Si no ha caido en el piso   
       legPosX = legVel*(cos(anguloAlfa))*cos(legAnguloBeta)*tiempoParabolicoLeg + init_legPosX;
       legPosY = posTemp;
       legPosZ = legVel*(cos(anguloAlfa))*sin(legAnguloBeta)*tiempoParabolicoLeg + init_legPosZ;
    }
    else{//Cayo en el piso 
         legPosY = legLado/4;//Dividido 4 dado que el leg tiene un escalamiento en y de 0.5, vuelve y lo nivela ensima del piso
         legVel *= desgaste;//Desgasta la velocidad un poco para que rebote menos cada vez
         legVel = -legVel;//Rebote
         
         init_legPosX = legPosX;//Pos inicial para cada rebote se actualiza y toma el valor de la final de la caida anterior
         init_legPosZ = legPosZ;
         init_legPosY = legPosY; 
         
         tiempoParabolicoLeg = 0.0;//Restaura el tiempo del movimiento parabolico para el siguiente lanzamiento
    }
    
    posTemp = artLegVel*(sin(anguloAlfa))*tiempoParabolicoArtLeg - (GRAVITY*tiempoParabolicoArtLeg*tiempoParabolicoArtLeg)/2 + init_artLegPosY ;
    if(posTemp - artLegRadio>= 0.0 ){         
       artLegPosX = artLegVel*(cos(anguloAlfa))*cos(artLegAnguloBeta)*tiempoParabolicoArtLeg + init_artLegPosX;
       artLegPosY = posTemp;
       artLegPosZ = artLegVel*(cos(anguloAlfa))*sin(artLegAnguloBeta)*tiempoParabolicoArtLeg + init_artLegPosZ;
    }
    else{
         artLegPosY = artLegRadio;
         artLegVel *= desgaste;
         artLegVel = -artLegVel;//Rebote
         
         init_artLegPosX = artLegPosX;
         init_artLegPosZ = artLegPosZ;
         init_artLegPosY = artLegPosY; 
         
         tiempoParabolicoArtLeg = 0.0;
    }
    
    posTemp = armVel*(sin(anguloAlfa))*tiempoParabolicoArm - (GRAVITY*tiempoParabolicoArm*tiempoParabolicoArm)/2 + init_armPosY ;
    if(posTemp - armLado/4>= 0.0 ){
       armPosX = armVel*(cos(anguloAlfa))*cos(armAnguloBeta)*tiempoParabolicoArm + init_armPosX;
       armPosY = posTemp;
       armPosZ = armVel*(cos(anguloAlfa))*sin(armAnguloBeta)*tiempoParabolicoArm + init_armPosZ;
    }
    else{
         armPosY = armLado/4; 
         armVel *= desgaste;
         armVel = -armVel;//Rebote
         
         init_armPosX = armPosX;
         init_armPosZ = armPosZ;
         init_armPosY = armPosY; 
         
         tiempoParabolicoArm = 0.0;
    }
    
    posTemp = artArmVel*(sin(anguloAlfa))*tiempoParabolicoArtArm - (GRAVITY*tiempoParabolicoArtArm*tiempoParabolicoArtArm)/2 + init_artArmPosY;
    if(posTemp - artArmRadio >= 0.0 ){
       artArmPosX = artArmVel*(cos(anguloAlfa))*cos(artArmAnguloBeta)*tiempoParabolicoArtArm + init_artArmPosX;
       artArmPosY = posTemp;
       artArmPosZ = artArmVel*(cos(anguloAlfa))*sin(artArmAnguloBeta)*tiempoParabolicoArtArm + init_artArmPosZ;
    }
    else{
         artArmPosY = artArmRadio; 
         artArmVel *= desgaste;
         artArmVel = -artArmVel;//Rebote
         
         
         init_artArmPosX = artArmPosX;
         init_artArmPosZ = artArmPosZ;
         init_artArmPosY = artArmPosY; 
         
         tiempoParabolicoArtArm = 0.0;
    }
         
   posTemp = headVel*(sin(anguloAlfa))*tiempoParabolicoHead - (GRAVITY*tiempoParabolicoHead*tiempoParabolicoHead)/2 + init_headPosY;
   if( posTemp - headRadio>= 0){
       headPosX = headVel*(cos(anguloAlfa))*cos(headAnguloBeta)*tiempoParabolicoHead + init_headPosX;
       headPosY = posTemp;
       headPosZ = headVel*(cos(anguloAlfa))*sin(headAnguloBeta)*tiempoParabolicoHead + init_headPosZ;
    }
    else{
         headPosY = headRadio;
         headVel = -headVel;
         headVel *= desgaste;
         
         init_headPosX = headPosX;
         init_headPosZ = headPosZ;
         init_headPosY = headPosY;
         
         tiempoParabolicoHead = 0.0;
    }
    
   //Se le suman deltaTiempoParabolico al tiempo para que sea mas rapido, para que muestre cada 0.001 seg donde esta en el dt=deltaTiempoParabolico
    tiempoParabolicoLeg += deltaTiempoParabolico; 
    tiempoParabolicoArtLeg += deltaTiempoParabolico; 
    tiempoParabolicoArm += deltaTiempoParabolico;
    tiempoParabolicoArtArm += deltaTiempoParabolico; 
    tiempoParabolicoHead += deltaTiempoParabolico;
    
   while ((clock() - tiempo) < 0.001*CLOCKS_PER_SEC)//Para que se vea fluido hacemos un delay de 0.001 seg no mas
   {
   }
}


//Retorna un random float entre manVelY y manVelY/2
float getRandomVelocidad(){
    float randomTemp = rand()/(float(RAND_MAX)+1);//Genera un random float entre 0.0 y 0.9999
    return  randomTemp*(manVelY-(manVelY/2.0))+(manVelY/2.0);    
}
//Asigna las velocidades iniciales randomicas para el movimiento parabolico de cada parte
void setVelocidades(){ 
    float tempDesechable = getRandomVelocidad();//La primera vez que se llama saca el mismo valor, despues si es random
    legVel =  getRandomVelocidad();
    artLegVel = getRandomVelocidad();
    armVel = getRandomVelocidad();
    artArmVel = getRandomVelocidad();
    headVel = getRandomVelocidad();   
}

//Implementa la fisica de la caida libre asignandole una nueva posicion al man para cada milisegundo de la caida.
void caidaLibre(){
  double tiempo= clock();

  manVelY += GRAVITY*tiempoCaidaLibre;
  manPosY -= GRAVITY*(tiempoCaidaLibre*tiempoCaidaLibre)*(0.5) + manVelY*tiempoCaidaLibre; 
  
  //Dividido 4 dado que los cubos tienen un escalamiento en y de 0.5
  if(manPosY - torsoLado/4 <= 0.0){//Colisiono contra el piso
              manPosY = 0.0;
              legPosY = legLado/4;//Para asegurarse que queden ensimita del piso y no queden por debajo del piso
              artLegPosY = artLegRadio;
              torsoPosY = torsoLado/4;
              armPosY = armLado/4;
              artArmPosY = artArmRadio;
              headPosY = headRadio;//--------------------------------------------------
              
              setVelocidades();
              //Se quiere que la altura a la que llegue el lanzamiento en el parabolico sea la mitad de la que se tiro
              GLfloat alturaMaxima = altura/2.0 ;
              
              anguloAlfa = asin( sqrt( ((alturaMaxima)*2.0*GRAVITY)) / (manVelY) );//Angulo de tiro para alcanzar un h maxima = alturaMaxima
              
              colisionoPisoCaida = true;
  }
  tiempoCaidaLibre = tiempoCaidaLibre + deltaTiempoCaida;
  //----------------"delay" para que sume 0.01 al tiempo y de verdad hayan pasado 0.01 seg-----------
  while ((clock() - tiempo) < 0.01*CLOCKS_PER_SEC)
    {
    }
  //----------------------------------------------------
                           
}

//Sabe que metodo llamar dependiendo dde cual ha colisionado contra el piso
void manejarFisica()
{
    if(!colisionoPisoCaida){
        caidaLibre();
    }
    else{
        movimientoParabolico();
    }
}

//Cambia el tiempo con el que se ven las animaciones, para que sea mas lento o mas rapido cada vez que se tire
void cambiarTiempo(int sumar){
    if(sumar){
       if(deltaTiempoCaida + 0.001 < 0.02){
           deltaTiempoCaida += 0.001;
       }
       
    }
    else{
       if(deltaTiempoCaida -0.001 > 0.005){
            deltaTiempoCaida -= 0.001;
       }
    }
    deltaTiempoParabolico = deltaTiempoCaida *40 ;
}



//Dibuja una malla de lineas de un tamaño de
void dibujarPiso(void){
    glPushMatrix();
    
	    GLfloat zExtent, xExtent;
	    
	    glLineWidth((GLfloat) 1.0);
	    
	    glBegin( GL_LINES );
	    glColor3f( 0.0, 0.7, 0.3 );
	    
	    zExtent = ESCALA_PISO * PASOS_Z_PISO;
	    for(GLfloat loopX = -PASOS_X_PISO; loopX <= PASOS_X_PISO; loopX+= 2 )
		{
	    	glVertex3f( loopX, 0.0, -zExtent );
	    	glVertex3f( loopX, 0.0,  zExtent );
		}
	
		glColor3f( 0.3, 0.0, 0.7 );
	    xExtent = ESCALA_PISO * PASOS_X_PISO;
	    
	    for(GLfloat loopZ = -PASOS_Z_PISO; loopZ <= PASOS_Z_PISO; loopZ+= 2)
		{
	    	glVertex3f( -xExtent, 0.0, loopZ );
	    	glVertex3f(  xExtent, 0.0, loopZ );
		}
	    glEnd();
    glPopMatrix();
}

//Dibuja un cubo de lado d y le pone una textura a algunas partes del cubo 
void dibujarTorsoConTextura(double d){
   glEnable( GL_TEXTURE_2D );
     
   //Quad 1 UP face
   glBindTexture( GL_TEXTURE_2D, textureTorsoFront );
   glBegin(GL_QUADS);
     glTexCoord2f(0.0, 1.0);
     glVertex3f (-d/2, d/2, -d/2);
     glTexCoord2f(1.0, 1.0);
     glVertex3f (-d/2, d/2, d/2);
     glTexCoord2f(1.0, 0.0);
     glVertex3f (d/2, d/2, d/2);
     glTexCoord2f(0.0, 0.0);
     glVertex3f (d/2, d/2, -d/2);
   glEnd();

    //Quad 2 Down Face
   glBindTexture( GL_TEXTURE_2D, textureTorsoBack);
   glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f (d/2, -d/2, -d/2);
        glTexCoord2f(1.0, 0.0);
        glVertex3f (d/2, -d/2, d/2);
         glTexCoord2f(1.0, 1.0);
        glVertex3f (-d/2, -d/2, d/2);
        glTexCoord2f(0.0, 1.0);
        glVertex3f (-d/2, -d/2, -d/2);
   glEnd ( );
   
   //Quad 3 Left face
   glBegin(GL_QUADS);
        glVertex3f (-d/2, d/2, -d/2);
        glVertex3f (-d/2, d/2, d/2);       
        glVertex3f (-d/2, -d/2, d/2);       
        glVertex3f (-d/2, -d/2, -d/2);
    glEnd ( );
    
    //Quad 4 Right Face
    glBindTexture( GL_TEXTURE_2D, textureTorsoUp );
    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 1.0);
        glVertex3f (d/2, d/2, d/2);
        glTexCoord2f(1.0, 1.0);
        glVertex3f (d/2, d/2, -d/2);
        glTexCoord2f(1.0, 0.0);
        glVertex3f (d/2, -d/2, -d/2);
        glTexCoord2f(0.0, 0.0);
        glVertex3f (d/2, -d/2, d/2);
    glEnd ( ); 
        
    //Quad 5 Front Face
    
    glBindTexture( GL_TEXTURE_2D, textureTorsoSide );
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0);
      glVertex3f (-d/2, d/2, d/2);
      glTexCoord2f(0.0, 0.0);
      glVertex3f (d/2, d/2, d/2);
      glTexCoord2f(1.0, 0.0);
      glVertex3f (d/2, -d/2, d/2);
      glTexCoord2f(1.0, 1.0);
      glVertex3f (-d/2, -d/2, d/2);
    glEnd ( );
        
    //Quad 6 Back Face
    glBindTexture( GL_TEXTURE_2D, textureTorsoSide);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0);
      glVertex3f (-d/2, d/2, -d/2);
      glTexCoord2f(0.0, 0.0);
      glVertex3f (d/2, d/2, -d/2);
      glTexCoord2f(1.0, 0.0);
      glVertex3f (d/2, -d/2, -d/2);
      glTexCoord2f(1.0, 1.0);
      glVertex3f (-d/2, -d/2, -d/2);
	glEnd ( );
	
  glDisable( GL_TEXTURE_2D );
}

//Dibuja el man, con cada parte
void dibujarMan(){
     glPushMatrix();  
     glLineWidth((GLfloat) 5.0); 
     
     glScalef(manSizeX,manSizeY,manSizeZ);
        
     //glColor3f( 0.9, 1.0, 0.0 );
     glColor3f( 0.1, 0.1, 0.1);
     glTranslatef(manPosX, manPosY, manPosZ);
     
     glRotatef((GLfloat) manAngleY, 0., 1., 0.);
     
     //Legs------------------------------------------
     glPushMatrix();
         glTranslatef(legPosX, legPosY, legPosZ);
         glScalef(1.0, 0.5, 0.5);
         glutSolidCube(legLado);     
     glPopMatrix();
     
     glPushMatrix();
         glTranslatef(legPosX, legPosY, -legPosZ);
         glScalef(1.0, 0.5, 0.5);
         glutSolidCube(legLado);     
     glPopMatrix();
     //------------------------------------------
      
      //Articulation LEGS----------------------------
      glPushMatrix();
          glTranslatef(artLegPosX, artLegPosY, artLegPosZ);
          glutSolidSphere(artLegRadio, 20, 20);
      glPopMatrix(); 
      
      glPushMatrix();
          glTranslatef(artLegPosX, artLegPosY, -artLegPosZ);
          glutSolidSphere(artLegRadio, 20, 20);
      glPopMatrix(); 
      //------------------------------------------
     
     glColor3f( 0.3, 0.3, 0.3 );
     //TORSO -------------------------------------
     glPushMatrix();
         glTranslatef(torsoPosX, torsoPosY, -torsoPosZ);
         glScalef(1.0, 0.5, 1.0);
         dibujarTorsoConTextura(torsoLado);
     glPopMatrix(); 
     //-------------------------------------------

     glColor3f( 0.71, 0.68, 0.58);
     //ARMS---------------------------------------
     glPushMatrix();
         glTranslatef(armPosX, armPosY, armPosZ);
         glScalef(1.3, 0.5, 0.5);
         glutSolidCube(armLado);     
     glPopMatrix();
     
     glPushMatrix();
         glTranslatef(armPosX, armPosY, -armPosZ);
         glScalef(1.3, 0.5, 0.5);
         glutSolidCube(armLado);     
     glPopMatrix();
     //-----------------------------------------------
     
      //Articulation ARMS----------------------------
      glPushMatrix();
          glTranslatef(artArmPosX, artArmPosY, artArmPosZ);
          glutSolidSphere(artArmRadio, 20, 20);
      glPopMatrix(); 
      
      glPushMatrix();
          glTranslatef(artArmPosX, artArmPosY, -artArmPosZ);
          glutSolidSphere(artArmRadio, 20, 20);
      glPopMatrix(); 
      //------------------------------------------
      
     //glColor3f( 0.9, 1.0, 0.0 );
     
     //HEAD---------------------------------------
     glPushMatrix();
         glColor3f( 0.8, 0.6, 0.5 );
         glTranslatef(headPosX, headPosY, headPosZ);
         glutSolidSphere(headRadio,20,20);  
     glPopMatrix();
     //-------------------------------------------
     
     glPopMatrix();
}


/*
//Renderiza un string en la pantalla 
void renderStrokeCharacter(float x, float y, float z, void *font,char *string)
{
  char *c;
  glPushMatrix();
  
  glColor3f( 1.0, 1.0, 1.0 );
      glTranslatef(x, y, z);
      for (c=string; *c != '\0'; c++) {
        glutStrokeCharacter(font, *c);
      }
  glPopMatrix();
}

//Dibuja las letras que aparecen en el fondo con los controles
void dibujarTextoControles(){  
     if(mostrarOcultarControles){ 
         glPushMatrix();
                 glScalef(0.08, 0.08, 0.4);
                 glLineWidth(3.0);
                 renderStrokeCharacter(-800,1500,-800,(void *)(int)GLUT_STROKE_ROMAN,"    Controles: ");
                 glLineWidth(1.0);
                 renderStrokeCharacter(-1300,1050,-800,(void *)(int)GLUT_STROKE_ROMAN,"Aparecer y desaparecer controles -> ' c '");
                 renderStrokeCharacter(-2500,850,-800,(void *)(int)GLUT_STROKE_ROMAN,"Man -> ' a ' y ' d ' para girar al man, ' w ' y ' s ' para aumentar o disminuir su altura");
                 renderStrokeCharacter( -2500,650,-800,(void *)(int)GLUT_STROKE_ROMAN,"Camara -> flechas para angulo, ' + ' y ' - ' para moverla hacia donde este mirando");
                 renderStrokeCharacter(-2700,450,-800,(void *)(int)GLUT_STROKE_ROMAN,"Animacion -> ENTER para dejarlo caer, con F1 se pone mas lenta la simulacion y con F2 mas rapida");
                 renderStrokeCharacter(-600,250,-800,(void *)(int)GLUT_STROKE_ROMAN,"Resetear man -> ' r '");
                 
         glPopMatrix();
     }
}
*/
// Dibujar toda la escena
void display(void)
{   
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
                     //PISO pero muy peyesito
                   /*  glColor3f(0.0, 0.0, 1.0);
                	glBegin(GL_QUADS);
                		glVertex3f(-100.0f, 0.0f, -100.0f);
                		glVertex3f(-100.0f, 0.0f,  100.0f);
                		glVertex3f( 100.0f, 0.0f,  100.0f);
                		glVertex3f( 100.0f, 0.0f, -100.0f);
                	glEnd();*/
   	               
                   glCallList(piso_display_list);
                   dibujarMan();    
               		//dibujarTextoControles();
                  
     glPopMatrix();
     glFlush ();
     glutSwapBuffers();
     
}
 
// Especificar los parametros de visualizacion
void visualizationParams(void)
{
	// Sistema de coordenadas de proyeccion
	glMatrixMode(GL_PROJECTION);
	// Inicializa el sistema de coordenadas de la proyeccion
	glLoadIdentity();

	// Especifica proyeccion perspectiva
	gluPerspective(angle,fAspect,0.1,2000);

	// Especifica el sistema de coordenadas del modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa el sistema de coordenadas del modelo
	glLoadIdentity();

	// Especifica la posicion del observador		  
	orientarCamara();		  
}

// Se llama cuando cambia el tamanyo de la ventana
void reshape(GLsizei w, GLsizei h)
{
	// Para prevenir division por 0
	if ( h == 0 ) h = 1;

	// Tamanyo del viewport
	glViewport(0, 0, w, h);
 
	// Aspect Ration
	fAspect = (GLfloat)w/(GLfloat)h;

	visualizationParams();
}

void idle(void){
     
     if(!comenzoAnimacion){
         if(manAngleYDelta){
                            manAngleY = manAngleY % 360 + manAngleYDelta;
         }
         if(manMoveYDelta){
                           if(manPosY + manMoveYDelta >12.)
                           manPosY += manMoveYDelta;                  
         }
     }
     else{
         manejarFisica();
     }
     if( manejarCamara() || manAngleYDelta || manMoveYDelta ){//Si hubo algun cambio para que repinte
                          display();
     }
}

// Responder a los eventos de teclas especiales
void keyboard(unsigned char key, int x, int y)
{
    switch(key){
            case 'w': manMoveYDelta = 0.5f; break;
            case 's': manMoveYDelta = -0.5f; break;
            case '+': deltaMoveCam = 3; break;
            case '-': deltaMoveCam = -3; break;          
            case 'a': manAngleYDelta = 2; break;
            case 'd': manAngleYDelta = -2; break;
            case 'c': mostrarOcultarControles =  !mostrarOcultarControles; break; 
            case 'r': init(); break;
            //ENTER  
            case 13: 
                 altura = manPosY;//Desde donde lo tiro
                 comenzoAnimacion = true; 
                 break;
    }
}

void keyboardRelease(unsigned char key, int x, int y)
{
    switch(key){
            case 'w': manMoveYDelta = 0.0f; break;
            case 's': manMoveYDelta = 0.0f; break;
            case '+': deltaMoveCam = 0; break;
            case '-': deltaMoveCam = 0; break;   
            case 'a': manAngleYDelta = 0; break;
            case 'd': manAngleYDelta = 0; break;   
    }
}

/*Se llama cada vez que se precione una tecla especial
 Asigna cuanto se va a ser el delta del angulo y del movimiento*/
void specialKeyPress(int key, int x, int y) {

	switch (key) {
           // MANEJAN CUANTO SE VA A MOVER Y GIRAR LA CAMARA 
		case GLUT_KEY_LEFT : deltaAngleCamXZ = -0.03f;break;
		case GLUT_KEY_RIGHT : deltaAngleCamXZ = 0.03f;break;
		case GLUT_KEY_UP : deltaAngleCamY = 0.03; break;
		case GLUT_KEY_DOWN : deltaAngleCamY = -0.03; break;
        case GLUT_KEY_F1 : cambiarTiempo(0) ;break;
        case GLUT_KEY_F2 : cambiarTiempo(1);break;
	}
}

//Se llama cada vez que se suelte una tecla especial
void specialKeyRelease(int key, int x, int y) {

	switch (key) {
           //Los vuelve 0.0 ya que la tecla que solto corresponde al angulo o al movimiento y si no esta undiendo
           //esa tecla entonces no hay movimiento ni giro 
		case GLUT_KEY_LEFT : deltaAngleCamXZ = 0.0f;break;
		case GLUT_KEY_RIGHT : deltaAngleCamXZ = 0.0f;break;
		case GLUT_KEY_UP : deltaAngleCamY = 0;break;
		case GLUT_KEY_DOWN : deltaAngleCamY = 0;break;
	}
}

// Programa Principal
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(700,700);
	
	glutCreateWindow("Falling Man");
	
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	
	glutReshapeFunc(reshape);
	
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardRelease);//se llama cuando la tecla se suelta
	glutSpecialFunc(specialKeyPress);
	glutSpecialUpFunc(specialKeyRelease);
	
	
	glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);
    
    init();
	glutMainLoop();
	return 0;
}
