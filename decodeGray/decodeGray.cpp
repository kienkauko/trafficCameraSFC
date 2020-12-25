#include "all_lib.hpp"
#include "jpeg_decode.hpp"
#include "rgb2gray.hpp"
#include "density.hpp"

#define IMAGE_SIZE 921600
#define MUY_SIZE 307200

int network_socket;
uint8_t gray5f[5*307200];								// 5 frames for background initiate 
uint8_t gray1f[307200];
//uint8_t *gray1f_p = gray1f;								// only 1 frame
uint8_t *gray5f_p = gray5f;
unsigned char buf_vel[921600];
uint8_t red[307200], green[307200], blue[307200];		// 307200 = 640x480
//uint8_t gray_a[5][307200];        						// pyramid level fixed
uint8_t confirm_bg = 0; 								// confirm background data has been sent successed!
uint8_t *pconfirm_bg = &confirm_bg;
float32_t muy[307200], var[307200];						// for density test only

void decode_gray(){
	jpeg_decode(buf_vel, red, green, blue);
    rgbtogray (red, green, blue, gray1f);
}

int main(){
    int i = 0;
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    { 
        perror("socket failed");
        exit(EXIT_FAILURE);
    } 
       
    
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(9002); 
       

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen");
        exit(EXIT_FAILURE);
    } 

/****create socket**********************/
network_socket = socket(AF_INET, SOCK_STREAM, 0);
if (network_socket < 0) printf("Error when create socket\n");
else printf("Successful create socket\n");
// specify an address for the socket
struct sockaddr_in server_address;
server_address.sin_family = AF_INET;
server_address.sin_port = htons(9003);
//server_address.sin_addr.s_addr = inet_addr(ip);
int connection_status = 0;
int reconnection_flag=0;
int connection_status_copy = 0;
	do{
		server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
		
		if(connection_status == -1){
			cout << "Fail to connect to localhost." << endl;
			cout << "Please enter public IP: " << endl;
			
			char ip[50];
			fgets(ip, 50, stdin);
			fflush(stdin);
			server_address.sin_addr.s_addr = inet_addr(ip);
			connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
			reconnection_flag=1;
		}
	}while(connection_status == -1);
	
int check_send_muy = 0;
int check_send_var = 0;
int check_send_gray = 0;
int check_recv = 0;

/**************************************/


//////////////////////////////////
//check received buffer//
new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

	while(1){
   		cout << "A connection has been established!" << endl;
		for (i; i < 5; i++){
	//send(new_socket, respon, strlen(respon), 0);.
			check_recv = recv(new_socket, buf_vel, IMAGE_SIZE,MSG_WAITALL); 
			jpeg_decode(buf_vel, red, green, blue);
			rgbtogray (red, green, blue, gray5f_p);
			gray5f_p+=307200;
			cout << " Done frame: " << i << endl;
			if (i == 4){
				gray5f_p = gray5f;
				background_initiate(gray5f_p, muy, var);
				cout << " Done background-int! " << endl;
				check_send_muy = send(network_socket, muy, MUY_SIZE, MSG_NOSIGNAL);
				check_send_var = send(network_socket, var, MUY_SIZE, MSG_NOSIGNAL);
				// announce that this will send background initiation files
				// start sending massage, thing got complicated here.......
				while (check_send_muy == -1 || check_send_var == -1){
					cout << "Coundn't connect to server Density to send muy, var \n";
					cout << "Trying to reconnect in 1s... \n";
					sleep(2);
					network_socket = socket(AF_INET, SOCK_STREAM, 0);
					connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
					cout << "Reconnection status: " << connection_status << endl;

					if (connection_status != -1){
						cout << "Reconnect successfully! \n";
						break;
					}
				}
				cout << "Server Density has accepted connection, connection status: " << connection_status << endl;
			}
		}
		///Try to print out JPEG image ///////////////
        check_recv = recv(new_socket, buf_vel, IMAGE_SIZE, MSG_WAITALL);
        
		if (check_recv == 0){
			cout << "Client has been stopped by some reasons, waiting for reconnection ... \n";
			new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    		continue;
		}
		        
//		FILE * check_buff;
//		check_buff = fopen("test_buff", "w");
//		fwrite(buf_vel,1,921600, check_buff);
//		fclose(check_buff);
		//////////////////////////////////////////////
		
		decode_gray();
		cout << "Start sending gray image, no more background \n";
		
		//send data section//////
		
		check_send_gray = send(network_socket, gray1f, sizeof(gray1f), MSG_NOSIGNAL);
		
		while(check_send_gray == -1){
			cout << "Coundn't connect to server Density \n";
			cout << "Trying to reconnect in 1s... \n";
			sleep(2);
			network_socket = socket(AF_INET, SOCK_STREAM, 0);
			connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
			if (connection_status != -1){
				cout << "Reconnect successfully! \n";
				cout << "Sending muy, var ...\n";
				send(network_socket, muy, MUY_SIZE, MSG_NOSIGNAL);
				send(network_socket, var, MUY_SIZE, MSG_NOSIGNAL);
				break;
			}
		}
	}
	close(network_socket);
	return 0;
}
	
