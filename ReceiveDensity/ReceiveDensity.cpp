#include "all_lib.hpp"


int main(){
		//int i = 0;
		int lan_do_so=0;
		uint32_t av_density=0;
		float time_process=0;
		int opt = 1;
		int check_recv_lando = 0;
		int check_recv_density =0;
		int check_recv_time = 0;
		//khoi tao socket
	  int socket_desc , new_socket , c;
	  
      struct sockaddr_in server;
	  
      //Create socket
      socket_desc = socket(AF_INET , SOCK_STREAM , 0);
      if (socket_desc == -1){
         printf("Could not create socket");
      }
     
      server.sin_family = AF_INET;
      server.sin_addr.s_addr = INADDR_ANY;
      server.sin_port = htons(9005);

      //Bind
     if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0){
       puts("bind failed");
       return -1;
     }

     puts("bind done");

     //Listen
     listen(socket_desc , 3);

      //Accept and incoming connection
      //puts("Waiting for incoming connections...");
      c = sizeof(struct sockaddr_in);
	  fflush(stdout);
    	/****************************************************************/
    	////nhan lien tuc
    new_socket = accept(socket_desc, (struct sockaddr *)&server,(socklen_t*)&c);
    puts("Connection accepted");
    
	while(1){
		
		check_recv_lando = recv(new_socket, &lan_do_so, sizeof(lan_do_so),MSG_WAITALL);
		check_recv_density = recv(new_socket, &av_density, sizeof(av_density), MSG_WAITALL);
		check_recv_time = recv(new_socket, &time_process, sizeof(time_process), MSG_WAITALL);
			
		if(check_recv_lando == 0 || check_recv_density == 0 || check_recv_time == 0){
			cout << "Client has been stopped by some reasons, waiting for reconnection ... \n";
    		new_socket = accept(socket_desc, (struct sockaddr *)&server,(socklen_t*)&c);
    		continue;
		}
		cout << "Lan do so: " << lan_do_so << endl;
		cout << "Density: " << av_density << "%" << endl;
		cout << "It took " << time_process << " seconds to process!" << endl;
	}
	return 0;
}
