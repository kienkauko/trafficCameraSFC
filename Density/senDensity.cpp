#include "all_lib.hpp"
#include "density.hpp"
//#include "receive.hpp"

#define SO_LAN_DO 10

int av_density_arr[SO_LAN_DO];
int lan_do = 0;

float32_t muy[307200], var[307200];
//uint8_t bin[307200]; 
uint8_t gray1f[307200];
int confirm_bg = 0;
int *pconfirm_bg = &confirm_bg;
int check_recv = 0;
int check_send_lando = 0, check_send_density = 0, check_send_time = 0;
uint32_t av_density;
uint32_t *p_av_density = &av_density;
int frame_count = 0;

struct timeval tv_main = {0};

void density(){
	av_density = findDensity_test(gray1f, muy, var);

	  // -------------------- Clear vector --------------------------------
    frame_count++;
	cout << "Frame: " <<frame_count << endl;
    cout << '-' << "Density: " << av_density << '%' << endl;
}

int main()
{
        
        //lan do so?
      	int lan_do_so = 0;
		int *p_lan_do_so = &lan_do_so;

		//thoi gian thuc hien
		float difference = 0;
		float *p_difference=&difference;
		
		//khoi tao socket de nhan du lieu
	  int socket_desc , new_socket , c; //receive data
	  int network_socket;		// send data
      struct sockaddr_in server , client;
	  struct timespec start_p, end_p; //measure time to process
      //Create socket
      socket_desc = socket(AF_INET , SOCK_STREAM , 0);
      if (socket_desc == -1)
      {
         printf("Could not create socket");
      }
	 // Forcefully attaching socket to port
//	 int opt = 1;
//      if (setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
//        perror("setsockopt");
//        exit(EXIT_FAILURE);
//      }
      
      //Prepare the sockaddr_in structure
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = INADDR_ANY;
      server.sin_port = htons(9003);

      //Bind
     if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
     {
       puts("bind failed");
       return -1;
     }

     puts("bind done");

     //Listen
    listen(socket_desc , 3);
    puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
    fflush(stdout);
    ///create socket for sending //////////////////
    network_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (network_socket < 0) 
		printf("Error when create socket\n");
	else printf("Successful create socket\n");
	
	// specify an address for the socket
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9005);
	//server_address.sin_addr.s_addr = inet_addr(ip);
	
	int connection_status = 0;
	char ipaddress[50];
	do{
		server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
		if(connection_status == -1){
			
			cout << "Fail to connect to localhost." << endl;
			cout << "Please enter public IP: " << endl;
			fgets( ipaddress, 50, stdin );
			fflush(stdin);
			server_address.sin_addr.s_addr = inet_addr(ipaddress);
			connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));
		}
	}while(connection_status == -1);

    /****************************************************************/
    ////nhan lien tuc////////////////////////////
	new_socket = accept(socket_desc, (struct sockaddr *)&server,(socklen_t*)&c);
    
    while(1){
    		///start receiving background innitiation/////////
    		if (confirm_bg == 0){
    			recv(new_socket, muy, 307200, MSG_WAITALL);
    			recv(new_socket, var, 307200, MSG_WAITALL);
    			cout << "Done background initiation \n";
				confirm_bg = 1; //confirm that background initiation has been completed.
    		}
    			
    		
    		clock_gettime( CLOCK_REALTIME, &start_p); // for cal time
    		
    		check_recv = recv(new_socket, gray1f, 307200, MSG_WAITALL);	//nhan image in  gray
    		if (check_recv == 0){
    			cout << "Client has been stopped by some reasons, waiting for reconnection ... \n";
    			new_socket = accept(socket_desc, (struct sockaddr *)&client,(socklen_t*)&c);
    			confirm_bg == 0;
    			continue;
    		}

    		cout << "Evething seems fine, lando number is: " << lan_do << endl;
    		density();		//chay ham density
    		av_density_arr[lan_do] = av_density;
    		++lan_do;
			if (lan_do < SO_LAN_DO ) continue;
			// Neu chua do du so lan thi chua gui anh ve server
				
			av_density = 0;
			for (int i = 0; i < SO_LAN_DO; ++i){
				av_density += av_density_arr[i];
			}
				
			av_density = av_density / SO_LAN_DO;
			clock_gettime( CLOCK_REALTIME, &end_p);
			
			lan_do_so++;
			cout << "Lan do: " << lan_do_so << endl;
			cout << "Final density: " << av_density << "%" << endl;
			
			
			// calculate time-taken
			difference = (end_p.tv_sec - start_p.tv_sec) + (float)
			(end_p.tv_nsec - start_p.tv_nsec)/1000000000.0d;
			cout << "It took " << difference << " seconds to process " << endl;
			
			//connect to send lan_do_so, av_density, time process
			
			cout << "Starting sending data to final boss \n";
			
			check_send_lando = send(network_socket, p_lan_do_so, sizeof(int), MSG_NOSIGNAL);
			check_send_density = send(network_socket, p_av_density, sizeof(uint32_t), MSG_NOSIGNAL);
			check_send_time = send(network_socket, p_difference, sizeof(float), MSG_NOSIGNAL);
				
			while (check_send_lando == -1 || check_send_density == -1 || check_send_time == -1){
				cout << "Mayday! Server down! Reconnect in 2s \n";
				sleep(2);
				network_socket = socket(AF_INET, SOCK_STREAM, 0);
				connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));				
				cout << "Reconnection status: " << connection_status << endl;
				if (connection_status != -1){
					cout << "Reconnect successfully! \n";
					break;
				}
				
   			}
   			cout << "Finish lando number: " << lan_do_so << endl;
			lan_do = 0;
    	}
    close(network_socket);
    return 0;
}
