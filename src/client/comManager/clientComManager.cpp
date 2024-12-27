#include "clientComManager.h" 

using namespace std;
namespace fs = std::filesystem;

// CONSTRUCTOR
clientComManager::clientComManager(){
 
};

// PRIVATE METHODS
void clientComManager::start_sockets()
{

    //sock_cmd used for the client to send commands like: download, upload, delete, list_server, list_server, exit, two-way communication
    if ((this->sock_cmd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening cmd socket\n";
    
    //sock_upload used for the client to upload files from inotify events syncs into the server
    if ((this->sock_upload = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening upload socket\n";

    //sock_fetch used for the client to download files from the server if synchronization needed
    if ((this->sock_fetch = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        cout << "ERROR opening fetch socket\n";
    
}

void clientComManager::connect_sockets(int port, hostent* server)
{
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;     
	serv_addr.sin_port = htons(port);    
	serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
	bzero(&(serv_addr.sin_zero), 8);  

    if (connect(this->sock_cmd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        cout << "ERROR connecting cmd socket\n";
    else
        cout <<"cmd socket connected\n";

    Packet handshake_packet = Packet::receive_packet(this->sock_cmd);
    if(handshake_packet.get_type() == Packet::COMM_PACKET){
        if (connect(this->sock_upload,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            cout << "ERROR connecting upload socket\n";
        else
            cout <<"upload socket connected\n";

        if (connect(this->sock_fetch,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
            cout << "ERROR connecting fetch socket\n";
        else
            cout <<"fetch socket connected\n";
    }

    // Send packet to communicate to main server that it is a client
    Packet ack_packet(Packet::COMM_PACKET, 1, 1, "", 0);
    ack_packet.send_packet(this->sock_cmd);
}

void clientComManager::close_sockets()
{
    close(this->sock_cmd);
    close(this->sock_fetch);
    close(this->sock_upload);
    cout << "All sockets closed,";
}

void clientComManager::upload()
{
    // Receive file path from terminal input
    string file_path;
    cout << "\nInsira o path do arquivo desejado: ";
    cin >> file_path;

    // Send packet signaling to server what file it wants to upload
    Packet upload_command = Packet(Packet::CMD_PACKET, Command::UPLOAD, 1, (file_path + "\n").c_str(), (file_path + "\n").length());
    upload_command.send_packet(this->sock_cmd);

    // Upload file to server
    FileTransfer::send_file(file_path, this->sock_upload);   
}

void clientComManager::download()
{
    // Receive file name from terminal input
    string file_name;
    cout << "\nInsira o nome do arquivo desejado: ";
    cin >> file_name;
    string packet_file_name = "/" + file_name + "\n";

    // Send packet signaling to server what file it wants to download
    Packet download_command = Packet(Packet::CMD_PACKET, Command::DOWNLOAD, 1, packet_file_name.c_str(), packet_file_name.length());
    download_command.send_packet(this->sock_cmd);
    FileTransfer::receive_file("../" + file_name, this->sock_cmd);
}

void clientComManager::delete_file()
{
    // Receive file name from terminal input
    string file_name;
    cout << "\nInsira o nome do file que deseja deletar: ";
    cin >> file_name;

    // Delete specified file from sync dir
    string file_path = "../src/client/sync_dir/" + file_name;
    this->file_manager->delete_file(file_path);
}

void clientComManager::list_server()
{
    // Sends a packet signaling the server to execute the function for listing file times
    string client_info = (get_username() + "\n" + to_string(this->sock_cmd));
    
    // Creates the packet with the command type for listing the times and sends packet
    Packet list_server_command = Packet(Packet::CMD_PACKET, Command::LIST_SERVER, 1, client_info.c_str(), client_info.length());
    list_server_command.send_packet(this->sock_cmd);
}

void clientComManager::receive_list_server_times()
{
    int client_socket = this->sock_cmd;

    while (true) {
        // Receives a packet from the server
        Packet received_packet = Packet::receive_packet(client_socket);

        // If the packet is not a DATA_PACKET, it means there are no more data to receive
        if (received_packet.get_type() != Packet::DATA_PACKET) {
            std::cout << this->username + " received all file times information" << std::endl;
            break;
        }

        // Extracts the payload from the packet
        std::string payload(received_packet.get_payload(), received_packet.get_length());

        // Splits the payload to extract the file path and times
        std::istringstream payload_stream(payload);
        int total_paths = 0, index = 0;
        std::string path, mtime, atime, ctime;

        // Reads the data from the payload
        if (std::getline(payload_stream, path, '\n') &&
            std::getline(payload_stream, mtime, '\n') && 
            std::getline(payload_stream, atime, '\n') && 
            std::getline(payload_stream, ctime, '\n') && 
            payload_stream >> total_paths && 
            payload_stream >> index ) {
            
            // Assuming path is the complete file path
            std::filesystem::path file_path(path);  // Converts the path to a `path` object

            // Logs the received information
            std::cout << "Received file times for: " << file_path.filename().string() << std::endl;
            std::cout << "Modification time (MTime): " << mtime << std::endl;
            std::cout << "Access time (ATime): " << atime << std::endl;
            std::cout << "Change/Creation time (CTime): " << ctime << std::endl << std::endl;

            // Check if all paths are received
            if (index + 1 == total_paths) {
                std::cout << "All files received." << std::endl;
                break;
                return;
            }
        } else {
            std::cerr << "Error: Invalid payload format." << std::endl;
            break;
        }
    }
    return;
}

void clientComManager::list_client()
{
    if (this->file_manager) {
        std::vector<std::string> files = this->file_manager->list_files();
        if (files.empty()) {
            cout << "O diretório sync_dir está vazio." << std::endl;
        } else {
            cout << "Arquivos no diretório sync_dir:\n";
            for (const std::string& file : files) {
                std::cout << "- " << file << std::endl;
            }
        }
    } else {
        cout << "Erro: file_manager não configurado.";
    }
}

void clientComManager::exit_client()
{
    Packet exit_command = Packet(Packet::CMD_PACKET, Command::EXIT, 1, "", 0);
    exit_command.send_packet(this->sock_cmd);
    this->close_sockets();
    std::cout << " shutting down... bye bye" << std::endl;
    exit(EXIT_FAILURE);
}

void clientComManager::get_sync_dir()
{
    //first erase everything that was in clint sync_dir, we dont want other clients files in new clients directory
    cout << this->file_manager->erase_dir("../src/client/sync_dir") << endl;
    // Send packet signaling server to execute get_sync_dir with client info (username and socket)
    string client_info = (get_username() + "\n" + get_hostname() + "\n");
    Packet get_sync_command = Packet(Packet::CMD_PACKET, Command::GET_SYNC_DIR, 1, client_info.c_str(), client_info.length());
    get_sync_command.send_packet(this->sock_cmd);
}

void clientComManager::receive_sync_dir_files()
{
    int client_socket = this->sock_cmd;

    while (true) {
        // Receive a packet
        Packet received_packet = Packet::receive_packet(client_socket);
        if (received_packet.get_type() == Packet::ERR and received_packet.get_seqn() == Command::EXIT){
            exit_client();
            break;
        }

        if (received_packet.get_type() != Packet::DATA_PACKET) {
            cout << this->username + " sync dir is empty" << endl;
            break;
        }

        // Extract the payload
        std::string payload(received_packet.get_payload(), received_packet.get_length());

        // Split the payload to extract path, total paths, and index
        std::istringstream payload_stream(payload);
        std::string path;
        int total_paths = 0, index = 0;

        if (std::getline(payload_stream, path, '\n') &&
            payload_stream >> total_paths && 
            payload_stream >> index) {
            // Log received information
            // std::cout << "Received path: " << path << " (Index " << index << " of " << total_paths << ")" << std::endl;

            size_t last_slash = path.find_last_of("/\\");
            std::string filename = (last_slash != std::string::npos) ? path.substr(last_slash) : path;
            
            //put the file in the queue
            this->file_manager->add_path(filename);

            // Receive the file using the extracted path
            // cout << "will store at: ../src/client/sync_dir" + filename << endl;
            FileTransfer::receive_file("../src/client/sync_dir" + filename, client_socket);

            // Check if all paths are received
            if (index + 1 == total_paths) {
                // std::cout << "All files received." << std::endl;
                break;
                return;
            }
        } else {
            std::cerr << "Error: Invalid payload format." << std::endl;
            break;
        }
    }
    return;
}

// PUBLIC METHODS

// This is the interface on client that will delegate each method based on commands.
void clientComManager::execute_command(Command command) {
    switch (command) {
        case Command::UPLOAD: {
            upload();
            break;
        }

        case Command::DOWNLOAD: {        
            download();
            break;
        }
        
        case Command::DELETE: {
            delete_file();
            break;
        }

        case Command::LIST_SERVER: {
            list_server();
            receive_list_server_times();
            break; 
        }

        case Command::LIST_CLIENT: {
            list_client();
            break;
        }

        case Command::EXIT: { 
            exit_client();
            break;
        }

        case Command::GET_SYNC_DIR: {
            get_sync_dir();
            receive_sync_dir_files(); 
            break;
        }

        default:
            return;
    }
}

int clientComManager::connect_client_to_server(int argc, char* argv[])
{
    int  port;
    struct hostent *server;
    char self_hostname[256];
    gethostname(self_hostname, sizeof(self_hostname));
    
	set_username(argv[1]);
    set_hostname(self_hostname);
	server = gethostbyname(argv[2]);
    port = atoi(argv[3]);

	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    
    // SOCKET
    start_sockets();

    // CONNECT
	connect_sockets(port,server);
    execute_command(Command::GET_SYNC_DIR); 
    this->file_manager->set_sockets(this->sock_cmd, this->sock_upload, this->sock_fetch);

    return 0;
}

void clientComManager::await_sync()
{
    Packet received_packet = Packet::receive_packet(this->sock_fetch);

    // CMD_PACKET == DELETE SYNC
    if (received_packet.get_type() == Packet::CMD_PACKET) {
        string file_name = strtok(received_packet.get_payload(), "\n");
        string sync_dir_path = "../src/client/sync_dir";
        string file_path = sync_dir_path + file_name;
        this->file_manager->add_path(file_name);
        
        this->file_manager->delete_file(file_path);

    // DATA_PACKET == DOWNLOAD SYNC
    }else if(received_packet.get_type() == Packet::DATA_PACKET){
        string file_name = strtok(received_packet.get_payload(), "\n");
        string sync_dir_path = "../src/client/sync_dir";
        string file_path = sync_dir_path + file_name;
        this->file_manager->add_path(file_name);

        FileTransfer::receive_file(file_path, this->sock_fetch);
        return;
    }
}

// GETTERS & SETTERS
std::string clientComManager::get_username(){ return this->username; }
void clientComManager::set_username(std::string username){ this->username = username; }
std::string clientComManager::get_hostname(){ return this->hostname; }
void clientComManager::set_hostname(std::string hostname){ this->hostname = hostname; }
void clientComManager::set_sock_cmd(int sock_cmd){this->sock_cmd = sock_cmd;}
void clientComManager::set_sock_upload(int sock_upload){this->sock_upload = sock_upload;}
void clientComManager::set_sock_fetch(int sock_fetch){this->sock_fetch = sock_fetch;}

// asks for the file that will be deleted and sends request with file and username
void clientComManager::send_delete_request(std::string file_name)
{
    // Send packet signaling to server what file it wants to delete.
    Packet delete_command = Packet(Packet::CMD_PACKET, Command::DELETE, 1, (file_name + "\n").c_str(), file_name.length());
    delete_command.send_packet(this->sock_cmd);
}
//Setter do file_manager
void clientComManager::set_file_manager(clientFileManager* fm) {
    this->file_manager = fm;
    cout << "settei o file_manager" <<endl;
}