

#include <iostream>
#include <getopt.h>
#include "idevicerestore.h"
#include "recovery.h"
#include "common.h"
#include "normal.h"
#include <signal.h>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include "stats.hpp"

#define USEC_PER_SEC 1000000

static struct option longopts[] = {
    { "ecid",       no_argument,       NULL, 'e' },
    { "times",      no_argument,       NULL, 't'},
    { "abort",      no_argument,       NULL, 'a'},
    { "statistics",           no_argument,       NULL, 's'},
    { "help",               no_argument,       NULL, 'h' },
    { NULL, 0, NULL, 0 }
};

void cmd_help(){
    printf("Usage: noncestatistics [OPTIONS]\n");
    printf("tool to get a lot of nonces from various iOS devices/versions\n\n");
    

    printf("  -h, --help             prints usage information\n");
    printf("  -e, --ecid ECID        manually specify ECID of the device. Uses any device if not specified\n");
    printf("  -t, --times amount     speficy how many NONCES are collected. If not specified it will collect nonces until you enter ctrl+c\n");
    printf("  -a, --abort            resets device to normal mode\n");
    printf("  -s, --statistics       print statistics from nonce file\n");
    printf("  FILE                   File to write nonces to (or read from when using '-s' or '--statistics')\n");
    printf("\n");
}

int64_t parseECID(const char *ecid){
    const char *ecidBK = ecid;
    int isHex = 0;
    int64_t ret = 0;
    
    while (*ecid && !isHex) {
        char c = *(ecid++);
        if (c >= '0' && c<='9') {
            ret *=10;
            ret += c - '0';
        }else{
            isHex = 1;
            ret = 0;
        }
    }
    
    if (isHex) {
        while (*ecidBK) {
            char c = *(ecidBK++);
            ret *=16;
            if (c >= '0' && c<='9') {
                ret += c - '0';
            }else if (c >= 'a' && c <= 'f'){
                ret += 10 + c - 'a';
            }else if (c >= 'A' && c <= 'F'){
                ret += 10 + c - 'A';
            }else{
                return 0; //ERROR parsing failed
            }
        }
    }
    
    return ret;
}

struct idevicerestore_client_t* client;
FILE *fp;
static int running = 1;

static void cancelNonceCollection(int signo){
    printf("\nUser cancelled nonce collection\n");
    if (running == 0) fclose(fp),exit(-1);
    running = 0;
}

int main(int argc, const char * argv[]) {
    
    bool only_abort = false;
    
    char *ecid = 0;
    int times = 0;
    int optindex = 0;
    int opt = 0;
    while ((opt = getopt_long(argc, (char* const *)argv, "he:t:as", longopts, &optindex)) > 0) {
        switch (opt) {
            case 'h': // long option: "help"; can be called as short option
                cmd_help();
                return 0;
            case 'e': // long option: "ecid"; can be called as short option
                ecid = optarg;
                break;
            case 't': // long option: "times"; can be called as short option
                times = atoi(optarg);
                break;
            case 'a': // long option: "abort"; can be called as short option
                only_abort = true;
                break;
            case 's': // long option: "statistics"; can be called ad short option
                if (argc < 2) {
                    std::cout << "You must specify a filename as last argument!" << std::endl;
                    cmd_help();
                    return -1;
                }
                cmd_statistics(argv[argc-1]);
                return 0;
                break;
            default:
                cmd_help();
                return -1;
        }
    }
    
    client = idevicerestore_client_new();
    

    if (check_mode(client) < 0 || client->mode->index == MODE_UNKNOWN ||
        (client->mode->index != MODE_DFU && client->mode->index != MODE_RECOVERY && client->mode->index != MODE_NORMAL)) {
        error("ERROR: Unable to discover device mode. Please make sure a device is attached.\n");
        return -1;
    }
    
    if (check_hardware_model(client) == NULL || client->device == NULL) {
        error("ERROR: Unable to discover device model\n");
        return -1;
    }
    
    info("Identified device as %s, %s ", client->device->hardware_model, client->device->product_type);
    if (!only_abort) {
        if (argc < 2) {
            std::cout << "You must specify a filename as last argument!" << std::endl;
            cmd_help();
            return -1;
        }
        const char *filename = argv[argc-1];
        
        switch (client->mode->index) {
            case MODE_NORMAL:
                info("in normal mode... Trying to get in recovery...\n");
                normal_enter_recovery(client);
                break;
            case MODE_DFU:
                info("in dfu mode... This mode is NOT supported!\n");
                return -1;
                break;
            case MODE_RECOVERY:
                info("in recovery mode... This is correct.\n");
                break;
                
            default:
                info("failed\n");
                error("ERROR: Device is in an invalid state\n");
                return -1;
        }
        info("\n");
        
        
        if (!ecid) {
            std::cout << "No ECID was specified. Checking if any device is connected." <<std::endl;
            if(get_ecid(client, &client->ecid)<0){
                std::cout << "It seems like no device is connected :(" <<std::endl;
                cmd_help();
                return -1;
            }
        }else{
            client->ecid = parseECID(ecid);
        }
        
        
        
        
        std::cout << "Getting nonce statistics for device with ECID: " << client->ecid << std::endl;
        
        
        fp = fopen(filename, "a");
        fprintf(fp, "ECID: %llx\nIdentified device as %s, %s \n", client->ecid, client->device->hardware_model, client->device->product_type);
        unsigned int noncesCreated = 0;
        int increment = 1;
        if (times==0) {
            times++;
            increment=0;
        }
        signal(SIGINT, cancelNonceCollection);
        
        recovery_client_free(client);
        for (int i=0; i<times && running; i+=increment) {
            unsigned char* nonce = NULL;
            int nonce_size = 0;
            
            while (running && recovery_get_ap_nonce(client, &nonce, &nonce_size)< 0) usleep(100);
            printf("%06u\t",++noncesCreated);
            info("ApNonce=");
            for (int i = 0; i < nonce_size; i++) {
                info("%02x", nonce[i]);
                fprintf(fp, "%02x", nonce[i]);
            }
            fprintf(fp, "\n");
            info("\n");
            free(nonce);
            
            if (!running) break;
            recovery_send_reset(client);
            recovery_client_free(client);
            usleep(USEC_PER_SEC*0.5);
        }
        std::cout << "Waiting for device to reboot..." << std::endl;
        
        recovery_client_free(client);
    }
    while (recovery_client_new(client) <0);
    std::cout << "Resetting autoboot..." << std::endl;
    recovery_set_autoboot(client, true);
    recovery_send_reset(client);
    std::cout << "Done" << std::endl;
    
    fclose(fp);
    
    return 0;
}
