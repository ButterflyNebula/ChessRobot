//Libraries
#include <iostream>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <cmath>
#define PI 3.14159265
#ifdef _WIN32
#define O_NOCTTY 0
#else
#include <termios.h>
#endif
#include <string>
using namespace std;

//Function Declarations
void drawboard();
void locdraw(int, int, int, int,int,int);
float distance(int,int,int,int);
float baseangle(int,int,float);
float scaledDistance(float,float);
int findAngles(float ,float , float&, float&);
float calculateBaseTargetNumber(float);
float calculateShoulderTargetNumber(float);
float calculateElbowTargetNumber(float, float);
float calculateElbowTargetNumber(float);
int maestroGetPosition(int, unsigned char);
int maestroSetTarget(int, unsigned char, unsigned short);
void setServostoNeutral(int);
float moveShoulderToPosition(float shoulderA, float elbowA);
float calculateTempElbowTarget();
void pickUpPiece(int, int, int, int);
void placePiece(int,int, int, int);
void openClaw(int);
void closeClaw(int);
string checkPostion(string);
int findStartPositionX(string);
int findStartPositionY(string);
int findEndPositionX(string);
int findEndPositionY(string);


//Constants
const float Length1=14.0;//cm
const float Length2=27.5;//cm
const float scale=3.2;//cm
const float endeffectorHeight= 14-10; //cm
 const float Servo_MG995 = 37.74;
const float Servo_MG995_SH = 37.74;
const float Servo_MG995_SH1 = 37.74;;
const float Servo_MG995_EL = 30.0;
const float Servo_MG995_EL2 = 10.0;
const float Servo_SG5010 = 37.74;

//Other
int fd = 0;
string answer="YES";

//Aray Parts
//0-Pawn
//1-Knight
//2-Bishop
//3-Rook
//4-Queen
//5-King
float pieceTypes[6]={1,2,3,4,5,6};
float pieceheight= 3;


//Main Loop
int main()
{
	string answer;
	answer="Y";
	const char * device = "\\\\.\\COM4";  //"\\\\.\\USBSER000"; // Windows, "\\\\.\\COM6" also works
	fd = open(device, O_RDWR | O_NOCTTY);
	if (fd == -1)
	{
		perror(device);
		return 1;
	}

	while(answer.compare("Y")==0){

		//Entering the Move
		cout << "Enter Move (example: c3-f5): " ;
		string movePosition;
		cin >> movePosition ;

		//Check the Move
		string move=checkPostion(movePosition);

		//Assign the Move to the Point Coordinates
		int coordinate1x=findStartPositionX(move);
		int coordinate1y=findStartPositionY(move);
		int coordinate3x=findEndPositionX(move);
		int coordinate3y=findEndPositionY(move);

	   //After Choosing the Locations
		cout<<"Chess Board\n";
		//Finding the Pololu Device On port


	   //Robot Coordinates
	   int robotOriginX=5; int robotOriginY=-3;


	   //Board Creation
		drawboard();
		locdraw(coordinate1x,coordinate1y,robotOriginX,robotOriginY,coordinate3x,coordinate3y);

		//Calculations
		setServostoNeutral(fd);
		pickUpPiece(coordinate1x,coordinate1y,robotOriginX,robotOriginY);
		placePiece(coordinate3x,coordinate3y,robotOriginX,robotOriginY);
		cout<<"\nDo you want to replay? TYPE Y or N " <<endl;
		cin>>answer;
		if (answer.compare("Y")!=0){
			cout<<"See you later!";
			break;
		}

	}
   close(fd);
}



//Functions

//Draws Board
void drawboard()
{

	cout<<" ";
	//for the letters at the top
	char letters [8]={'a','b','c','d','e','f','g','h'};
	for(int c=0; c<8;c++){
		 cout<<" ";
		 cout<<letters[c];
		 cout<<" ";
		 if(c==7){
			 cout<<"\n";
		   }
	   }
	//for the x coordinates
	for(int i = 1; i < 9; i++)
	{
		if (i < 0)
			cout<<" "<<i;
		else
			cout<<"  "<<i;

	}
	cout<<endl;

}


//Draws Point Locations
void locdraw (int x, int y, int m, int n,int p, int o){
	//for the y coordinates
    for(int i = 7; i > -5; i=i-1)
        {

            cout<<(char)(i + 49);
            for(int j = 1; j < 9; j++)
            //starting position
            if(i == y - 1 && j == x)
                cout<<" x ";
            //Robot position
            else if(i == n - 1 && j == m)
                cout<<" + ";
            //End Position
            else if (i==o-1 && j==p){
            	cout<<" & ";
            }
            else
                cout<<" . ";

            cout<<(char)(i + 49)<<endl;
        }
}



//Finds the distance between the two point
float distance(int a,int b,int c, int d){
	int equationpart1=(a-c)*(a-c);
	int equationpart2=(b-d)*(b-d);
	int group1= equationpart1+equationpart2;
	float unscaledanswer= sqrt(group1);
	cout<<"The distance is ";
	cout<<unscaledanswer;
	return unscaledanswer;

}



//Finds the base angle
float baseangle (int a,int c,float distance){
	float hypotenuse=distance;
	// to solve for the angle
	int opposite=abs(c-a);
	float sin_definition=opposite/hypotenuse;
	float radians= asin(sin_definition);
	float degrees= radians*180/PI;
	if(a>5){
		degrees=-degrees;
	}
	cout<<"\nThe angle is ";
	cout<<degrees;
	return degrees;
}



//Finds the actual distance on the real board not the coordinate system
float scaledDistance(float unscaled,float scale){
	float actualDistance=unscaled*scale;
	cout<<"\nThe actual distance is ";
	cout<<actualDistance;
	return actualDistance;
}




// Gets the position of a Maestro channel.
// See the "Serial Servo Commands" section of the user's guide.
int maestroGetPosition(int fd, unsigned char channel)
{
	unsigned char command[] = {0x90, channel};
	if(write(fd, command, sizeof(command)) == -1)
	{
	perror("error writing");
	return -1;
	}
	unsigned char response[2];
	if(read(fd,response,2) != 2)
	{
	perror("error reading");
	return -1;
	}
	return response[0] + 256*response[1];
}





// Sets the target of a Maestro channel.
// See the "Serial Servo Commands" section of the user's guide.
// The units of 'target' are quarter-microseconds.
int maestroSetTarget(int fd, unsigned char channel, unsigned short target)
{
	unsigned char command[] = {0x84, channel, target & 0x7F, target >> 7 & 0x7F};
	if (write(fd, command, sizeof(command)) == -1)
	{
	perror("error writing");
	return -1;
	}
	return 0;
}



/**
 * This command will return the error number on the Pololu Maestro Board
 * The error number is made of two bytes. so the response needs to add the returned
 * bytes together.
 *
 * To get the error we have to write 0xA1 to the controller to set it into error mode
 * Then read the error
 *
 * @param int fd - The file descriptor to the device
 *
 * @returns int - The number that represents the Error. See Pololu documentation for error numbers
 */
int maestroGetError(int fd)
{
    unsigned char command[] = { 0xA1 };
    if (write(fd, command, sizeof(command)) == -1)
    {
        perror("error getting Error");
        return -1;
    }

    unsigned char response[2];
    if(read(fd,response,1) != 1)
    {
        perror("error reading");
        return -1;
    }

    //Helpful for debugging
    //printf("Error first: %d\n", response[0]);
    //printf("Error secon: %d\n", response[1]);

    return (int)sqrt(response[0] + 256*response[1]);
}






//Finds the parts theta and phi of the shoulder and elbow angles
int findAngles(float distance,float pieceHeight, float &thetaA, float &phiA){

	float targetLocationX=distance;
	float targetLocationY=pieceHeight+endeffectorHeight;
	float deltaX=0.2;//cm
	float deltaY=0.2;//cm
	float x;
	float y;
	int foundAngles = 0;
	double theta = thetaA * 3.14/180;
	double phi = phiA * 3.14/180;;

	cout<<"\n Target x is: ";
	cout<<targetLocationX;
	cout<<"\n Target y is: ";
	cout<<targetLocationY;

	cout<<"\n Starting theta  is: ";
	cout<<theta;
	cout<<" Starting Phi is ";
	cout<<phi;

	//Algorithm
	while(theta > 0){

			while(phi<3.14){
				x=(Length1*cos(theta))+(Length2*sin(phi));
				y=(Length1*sin(theta))- (Length2*cos(phi));

					if(abs(targetLocationY-y)<=deltaY&& abs(targetLocationX-x)<=deltaX){
						cout<<"\n FOUND SOLUTION \n ";
						foundAngles = 1;
						break;
					}
					else {
						phi = phi + .0125;
					}
				}
		if(abs(targetLocationY-y)<=deltaY && abs(targetLocationX-x)<=deltaX){
			break;
			}
		else{
			theta=theta - .0125;
			phi = 0;
			}
	}

	cout<<"\n x is: ";
	cout<<x;
	cout<<" y is: ";
	cout<<y;
	cout<<" Theta is: ";
	cout<<theta;
	cout<<" Phi is: ";
	cout<<phi;

	cout<<"\n Cos(Theta) is: ";
	cout<<cos(theta);


	thetaA = theta * 180/3.14;
	phiA = phi * 180/3.14;

	if(theta <= 0){
        foundAngles = 0;
		cout<<"No solution Found: error";
	}
	return foundAngles;
}


//Converts the angle found in baseangle function to a target for the Base Servo
float calculateBaseTargetNumber(float angle)
{
    float angleR = angle * 3.14 /180;
	cout<<"\n Base Angle in Degrees ";
		cout<< angle;


	cout<<"\n Base Angle in Radians ";
    cout<< angleR;

	float targetNumber = 6000 + angle * Servo_MG995;
	return targetNumber;
}


//Converts the angle found in findAngles to a target for the Shoulder
float calculateShoulderTargetNumber(float angle){
	float targetNumber;
	if(angle<125){
		 targetNumber=6000-((125-angle)*Servo_MG995_SH);
		 cout<<" Servo_MG995_SH " ;
		 cout<<Servo_MG995_SH;
		 cout<<" applied for ";
		 cout<<angle;
		 cout<<"\n ";
	}
	else if (angle==125){
		targetNumber=6000;
	}
	else{
		 cout<<" Servo_MG995_SH " ;
		 cout<<Servo_MG995_SH1;
		 cout<<" applied for ";
		 cout<<angle;
		 cout<<"\n ";
		targetNumber=6000+((angle-125)*Servo_MG995_SH1);
	}
	cout<<"Shoulder Targer is " ;
	cout<<targetNumber;
	cout<<"\n ";
	return targetNumber;
}

//Converts the angle found in findAngles to a target for the Elbow
float calculateElbowTargetNumber(float angle, float shoulderAngle){
	float tempTargetAngle;

	if (shoulderAngle > 90)
	{
		tempTargetAngle = angle - (90 - (180 - shoulderAngle));
	}
	else if (shoulderAngle == 90)
	{
		tempTargetAngle = angle;
	}
	else
	{
		tempTargetAngle = angle + 90 - shoulderAngle;
	}

	cout<<"\nThe Final Elbow Angle is ";
	cout<<tempTargetAngle;

	float targetNumber;

	if(tempTargetAngle<65){
				 targetNumber = 6000 + (65 - tempTargetAngle) * Servo_MG995_EL ;
				 cout<<" Servo_MG995_EL " ;
				 cout<<Servo_MG995_EL;
				 cout<<" applied for ";
				 cout<<tempTargetAngle;
				 cout<<"\n ";
			}
			else if (tempTargetAngle==65){
				targetNumber=6000;
			}
			else{
				targetNumber=6000-((tempTargetAngle-65)*Servo_MG995_EL2);
				 cout<<" Servo_MG995_EL2 " ;
				 cout<<Servo_MG995_EL2;
				 cout<<" applied for ";
				 cout<<tempTargetAngle;
				 cout<<"\n ";
			}

		/*	if(tempTargetAngle<65){
			 targetNumber=6000+((65-tempTargetAngle)*Servo_MG995);
		}
		else if (tempTargetAngle==65){
			targetNumber=6000;
		}
		else{
			targetNumber=6000-((tempTargetAngle-65)*Servo_MG995);
		}  */
		cout<<"\n The elbow Servo will go to: ";
		cout<<targetNumber;
		cout<<"\n ";
		return targetNumber;
}


float calculateElbowTargetNumber(float angle){
	float tempTargetAngle = angle;



	float targetNumber;

	if(tempTargetAngle<65){
				 targetNumber = 6000 + tempTargetAngle *Servo_MG995_EL ;
				 cout<<" Servo_MG995_EL " ;
				 cout<<Servo_MG995_EL;
				 cout<<" applied for ";
				 cout<<tempTargetAngle;
				 cout<<"\n ";
			}
			else if (tempTargetAngle==65){
				targetNumber=6000;
			}
			else{
				targetNumber=6000-((tempTargetAngle-65)*Servo_MG995_EL2);
				cout<<" Servo_MG995_EL2 " ;
				cout<<Servo_MG995_EL2;
				cout<<" applied for ";
				cout<<tempTargetAngle;
				cout<<"\n ";
			}

		/*	if(tempTargetAngle<65){
			 targetNumber=6000+((65-tempTargetAngle)*Servo_MG995);
		}
		else if (tempTargetAngle==65){
			targetNumber=6000;
		}
		else{
			targetNumber=6000-((tempTargetAngle-65)*Servo_MG995);
		}  */
		cout<<"\n The elbow servo will go to: ";
		cout<<targetNumber;
		return targetNumber;
}


//Sets all the servos to their "Neutral" Position
void setServostoNeutral(int fd){
	//Base Angle

	//Shoulder
	maestroSetTarget(fd, 0, 6000);
	sleep(1);
	//Elbow
	maestroSetTarget(fd, 2, 6000);
	sleep(1);
	//Hand
	maestroSetTarget(fd, 6, 6000);
	sleep(1);
	maestroSetTarget(fd, 4, 6000);
	sleep(1);
}

//Opens the claw
void openClaw(int fd){
	maestroSetTarget(fd, 6, 8800);
}

//Closes the claw
void closeClaw(int fd){
	maestroSetTarget(fd, 6, 6000);
}

float moveShoulderToPosition(float shoulderA, float elbowA)
{
	float tempElbowA = 0;
    float elbowTarget = 0;
    float shoulderTarget = 0;
    float tempShoulderA = 120;

    if (shoulderA <= 120)
    {
    	tempShoulderA = 120;
    	while (tempShoulderA > shoulderA)
    	{
    		shoulderTarget = calculateShoulderTargetNumber(tempShoulderA);
    		tempElbowA = 185 - tempShoulderA;
    		elbowTarget = calculateElbowTargetNumber(tempElbowA);
    		maestroSetTarget(fd, 2, elbowTarget);
    		maestroSetTarget(fd, 0, shoulderTarget);
    		tempShoulderA = tempShoulderA - 10;
    	}
    }

	shoulderTarget = calculateShoulderTargetNumber(shoulderA);
	cout<<"\n Shoulder Number is: ";
	cout<<shoulderTarget;

	maestroSetTarget(fd, 0, shoulderTarget);
    return(elbowTarget);
}

/* float moveShoulderToPosition(float shoulderA, float elbowA)
{
	float tempElbowA = 0;
    float elbowTarget = 0;
    float shoulderTarget = 0;


	if (shoulderA < 120)
	{
		tempElbowA = 2 * shoulderA;
		if (tempElbowA > 120)
		{
			tempElbowA = 120;
		}
		if (tempElbowA < 120)
		{
			tempElbowA = 60;
		}
		elbowTarget = calculateElbowTargetNumber(tempElbowA);
		cout<<"\n Temp Elbow Number is: ";
		cout<<elbowTarget;
		cout<<"\n Temp Elbow Angle is: ";
		cout<<tempElbowA;
		maestroSetTarget(fd, 2, elbowTarget);
	}
	shoulderTarget = calculateShoulderTargetNumber(shoulderA);
	cout<<"\n Shoulder Number is: ";
	cout<<shoulderTarget;

	maestroSetTarget(fd, 0, shoulderTarget);
    return(elbowTarget);
}   */

//Checks if the move is valid
string checkPostion(string movePosition) {
	while(true){
				std::string startPositionLetter=movePosition.substr(0,1);
				std::string startPositionNumber=movePosition.substr(1,1);
				std::string endPositionLetter=movePosition.substr(3,1);
				std::string endPositionNumber=movePosition.substr(4,1);


				if ( (  startPositionLetter.find("a")!= string::npos||	startPositionLetter.find("b")!= string::npos||
						startPositionLetter.find("c")!= string::npos||	startPositionLetter.find("d")!= string::npos||
						startPositionLetter.find("e")!= string::npos||	startPositionLetter.find("f")!= string::npos||
						startPositionLetter.find("g")!= string::npos||	startPositionLetter.find("h")!= string::npos
					)
					&&
				    (   startPositionNumber.find("1")!= string::npos||	startPositionNumber.find("2")!=string::npos ||
						startPositionNumber.find("3")!= string::npos||	startPositionNumber.find("4")!= string::npos||
						startPositionNumber.find("5")!= string::npos||	startPositionNumber.find("6")!= string::npos||
						startPositionNumber.find("7")!= string::npos||	startPositionNumber.find("8")!= string::npos
					 )
					 &&
					 (  endPositionLetter.find("a")!= string::npos||	endPositionLetter.find("b")!=string::npos||
						endPositionLetter.find("c")!= string::npos||	endPositionLetter.find("d")!= string::npos||
						endPositionLetter.find("e")!= string::npos||	endPositionLetter.find("f")!= string::npos||
						endPositionLetter.find("g")!= string::npos||	endPositionLetter.find("h")!= string::npos
					 )
					 &&
					 (  endPositionNumber.find("1")!= string::npos||	endPositionNumber.find("2")!=string::npos||
						endPositionNumber.find("3")!= string::npos||	endPositionNumber.find("4")!= string::npos||
						endPositionNumber.find("5")!= string::npos||	endPositionNumber.find("6")!= string::npos||
						endPositionNumber.find("7")!= string::npos||	endPositionNumber.find("8")!= string::npos
					 )
					)
				{

					break;
				}
				else {
					cout<<"Invalid Position Please Re-enter: ";
					cin >> movePosition;
				}
			}
	return movePosition;
}

//Finds the start Position x coordinate
int findStartPositionX(string movePosition){
	std::string startPositionLetter=movePosition.substr(0,1);
	int coordinate;
	if (startPositionLetter.compare("a")==0){
		coordinate=1;
	}
	else if (startPositionLetter.compare("b")==0){
		coordinate=2;
	}
	else if (startPositionLetter.compare("c")==0){
			coordinate=3;
		}
	else if (startPositionLetter.compare("d")==0){
			coordinate=4;
		}
	else if (startPositionLetter.compare("e")==0){
			coordinate=5;
		}
	else if (startPositionLetter.compare("f")==0){
			coordinate=6;
		}
	else if (startPositionLetter.compare("g")==0){
			coordinate=7;
		}
	else {
			coordinate=8;
		}

	return coordinate;
}

//Finds the start Position y coordinate
int findStartPositionY(string movePosition){
	std::string startPositionNumber=movePosition.substr(1,1);
	int coordinate;

	if (startPositionNumber.compare("1")==0){
			coordinate=1;
		}
		else if (startPositionNumber.compare("2")==0){
			coordinate=2;
		}
		else if (startPositionNumber.compare("3")==0){
				coordinate=3;
			}
		else if (startPositionNumber.compare("4")==0){
				coordinate=4;
			}
		else if (startPositionNumber.compare("5")==0){
				coordinate=5;
			}
		else if (startPositionNumber.compare("6")==0){
				coordinate=6;
			}
		else if (startPositionNumber.compare("7")==0){
				coordinate=7;
			}
		else {
				coordinate=8;
			}

		return coordinate;
}

//Finds the end Position x coordinate
int findEndPositionX(string movePosition){
	std::string endPositionLetter=movePosition.substr(3,1);
	int coordinate;

	if (endPositionLetter.compare("a")==0){
			coordinate=1;
		}
		else if (endPositionLetter.compare("b")==0){
			coordinate=2;
		}
		else if (endPositionLetter.compare("c")==0){
				coordinate=3;
			}
		else if (endPositionLetter.compare("d")==0){
				coordinate=4;
			}
		else if (endPositionLetter.compare("e")==0){
				coordinate=5;
			}
		else if (endPositionLetter.compare("f")==0){
				coordinate=6;
			}
		else if (endPositionLetter.compare("g")==0){
				coordinate=7;
			}
		else {
				coordinate=8;
			}

		return coordinate;
}

int findEndPositionY(string movePosition){
	std::string endPositionNumber=movePosition.substr(4,1);
    int coordinate;

    if (endPositionNumber.compare("1")==0){
    			coordinate=1;
    		}
	else if (endPositionNumber.compare("2")==0){
		coordinate=2;
	}
	else if (endPositionNumber.compare("3")==0){
			coordinate=3;
		}
	else if (endPositionNumber.compare("4")==0){
			coordinate=4;
		}
	else if (endPositionNumber.compare("5")==0){
			coordinate=5;
		}
	else if (endPositionNumber.compare("6")==0){
			coordinate=6;
		}
	else if (endPositionNumber.compare("7")==0){
			coordinate=7;
		}
	else {
			coordinate=8;
		}

	return coordinate;
}

void pickUpPiece(int coordinate1x,int coordinate1y, int robotOriginX, int robotOriginY)
{
	float theta = 150;
	float phi = 0;
    int angleFound = 0;

	float unscaledanswer= distance(coordinate1x,coordinate1y,robotOriginX,robotOriginY);
	float base_angle=baseangle(coordinate1x,robotOriginX,unscaledanswer);
	float actualDistance= scaledDistance(unscaledanswer,scale);
	angleFound = findAngles(actualDistance,pieceheight, theta, phi);

	if (angleFound > 0)
	{
			float baseAngleTarget=calculateBaseTargetNumber(base_angle);
			float elbowTarget=calculateElbowTargetNumber(phi, theta);

				   //Print Text
			cout<<"\nThe base Angle's Target is: ";
			cout<<baseAngleTarget;
			cout<<"\nTheta is ";
			cout<<theta;
			cout<<"\nPhi is ";
			cout<<phi;


			//Setting Servos
			setServostoNeutral(fd);

			openClaw(fd);
			sleep(3);
			maestroSetTarget(fd, 4, baseAngleTarget);
			sleep(3);
			moveShoulderToPosition(theta, phi);
			sleep(3);
			maestroSetTarget(fd, 2, elbowTarget);
			sleep(3);
			closeClaw(fd);
			sleep(3);
		 //   maestroSetTarget(fd, 2, 6000);
		  //  sleep(3);
			setServostoNeutral(fd);
	}

}


void placePiece(int coordinate3x,int coordinate3y, int robotOriginX, int robotOriginY)
{
	float theta = 150;
    float phi = 0;
    int foundAngle = 0;

		float unscaledanswer= distance(coordinate3x,coordinate3y,robotOriginX,robotOriginY);
		float base_angle=baseangle(coordinate3x,robotOriginX,unscaledanswer);
		float actualDistance= scaledDistance(unscaledanswer,scale);
		foundAngle = findAngles(actualDistance,pieceheight, theta, phi);
		if (foundAngle > 0)
		{
				float baseAngleTarget=calculateBaseTargetNumber(base_angle);
				float elbowTarget=calculateElbowTargetNumber(phi, theta);

					   //Print Text
				cout<<"\nThe base Angle's Target is: ";
				cout<<baseAngleTarget;
				cout<<"\nTheta is ";
				cout<<theta;
				cout<<"\nPhi is ";
				cout<<phi;


				 //Setting Servos
				maestroSetTarget(fd, 4, baseAngleTarget);
				sleep(3);
				moveShoulderToPosition(theta, phi);
				sleep(3);
				maestroSetTarget(fd, 2, elbowTarget);
				sleep(3);
				openClaw(fd);
				sleep(4);
				setServostoNeutral(fd);
		}
}
