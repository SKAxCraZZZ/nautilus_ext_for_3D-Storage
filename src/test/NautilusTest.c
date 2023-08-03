

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <glib.h>
#include <sys/socket.h>
#include <nautilus-extension.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <pthread.h>
#include <locale.h>

#define ADDRESS "/tmp/CoreFxPipe_IconStateNotifierPipe"

/*\
|*|
|*| BUILD SETTINGS
|*|
\*/

#ifdef ENABLE_NLS
#include <glib/gi18n-lib.h>
#include <sys/socket.h>
#include <nautilus-extension.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>

#define I18N_INIT() \
    bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
#else
#define _(STRING) ((char * ) STRING)
#define I18N_INIT()
#endif


/*\
|*|
|*| GLOBAL TYPES AND VARIABLES
|*|
\*/

int сurrentEmblem = 0;
static char *emblems[] = {"","loadedstore", "outdatedstore","notsentstore","editedstore","abortedstore","","","conflictstore"};

typedef struct {
	GObject parent_slot;
} NautilusTest;

typedef struct {
	GObjectClass parent_slot;
} NautilusTestClass;

static GType provider_types[1];
static GType nautilus_test_type;
static GObjectClass *parent_class;

/*\
|*|
|*| FUNCTIONS
|*|
\*/

int ChangeFileEmblem(NautilusFileInfo *file, int iconIndex);

static int request_state(char *path)
{
    int sock = 0;
    int data_len = 0;
    struct sockaddr_un remote;

    static const char *socket_path = "/tmp/CoreFxPipe_PilotInfoPipeshethpfmfs.3D-Storage20170614";
    unsigned int s_recv_len = 200;
    unsigned int s_send_len = 100;

    int uni = 1;
    char recv_msg[s_recv_len];
    char send_msg[s_send_len];

    memset(recv_msg, 0, s_recv_len * sizeof(char));
    memset(send_msg, 0, s_send_len * sizeof(char));

    if ((sock = socket(uni, SOCK_STREAM, 0)) == -1)
    {
        printf("Client: Error on socket() call \n");
    }

    remote.sun_family = uni;
    strcpy(remote.sun_path, socket_path);
    data_len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    printf("Client: Trying to connect... \n");
    if (connect(sock, (struct sockaddr*)&remote, data_len) == -1)
    {
        printf("Client: Error on connect call \n");
    }
    printf("Client: Connected \n");
    send_msg<path;

    if (send(sock, (char*)path, strlen(path) * sizeof(char), 0) == -1)
    {
        printf("Client: Error on send \n");
    }
    memset(send_msg, 0, s_send_len * sizeof(char));
    memset(recv_msg, 0, s_recv_len * sizeof(char));

    if ((data_len = recv(sock, recv_msg, s_recv_len, 0)) > 0)
    {
        printf("Client: Data received upd: %s \n", recv_msg);
        printf(recv_msg);
        close(sock);                
        int result;
        result = atoi(recv_msg);
        return result;
    }
    else
    {
        if (data_len < 0)
        {
            printf("Client: Error on recv() call \n");
        }
        else
        {
            printf("Client: Server socket closed \n");
            close(sock);
        }
        return 0;

    }
}

void* ListenSocket(void *arg)
{
    struct sockaddr_un ca;
    int ca_len,d1,lenght,i;
    FILE *fp;
    unsigned char c,c2,c3;
    unsigned char b1,b2;
    int d = (int)arg;

    char* filePrefix = "file://";
    char *uri;
    int iconindex;

    while(TRUE)
    {
        if ( listen ( d, 5) < 0 )
        {
            perror ("server: listen"); exit (1);
            printf(">>exit\n");
        }

        if (( d1 = accept ( d, &ca, &ca_len)) < 0 )
        {
            perror ("server: accept"); exit (1);
        }
        /* ------------------------------------------ */
        printf(">>reading\n");

        fp = fdopen (d1, "r");
        int b;
        for (b = 0; b < 2; b++)
        {
            c=fgetc(fp);
            c2=fgetc(fp);

            lenght= c * 256 + c2;
            printf(">>> lenght - %i \n", lenght);
            i=0;

            unsigned char bytes[lenght];
            while (i != lenght)
            {
                c3=fgetc(fp);
                bytes[i] = c3;
                i++;
            }

            int n = sizeof(bytes);
            printf(">> char lengh bytes - %i \n", n);
            char chars[n +1];
            memcpy(chars, bytes, n);
            chars[n] = '\0';


            if (strlen(chars)>2)
            {
                uri = malloc(strlen(filePrefix)+strlen(chars)+1);
                strcpy(uri, filePrefix );
                strcat(uri, chars);
            }
            else
            {
                printf(">>>index - %s \n", chars);
                iconindex = atoi(chars);
            }
        }

        printf(">>end reading\n");
        printf(">>> result index - %i \n", iconindex);

        printf(">>> result uri - %s \n", uri);
        NautilusFileInfo *file = nautilus_file_info_lookup_for_uri(uri);
        if (file != NULL)
        {
            nautilus_file_info_invalidate_extension_info(file);
            ChangeFileEmblem(file, iconindex);
        }
    }

}

void FileStateListner()
{
    int s,i, d,len;
    struct sockaddr_un sa;
    
    if((d = socket (1, SOCK_STREAM, 0)) < 0)
    {
     perror ("client: socket"); exit (1);
    }

    sa.sun_family = 1;
    strcpy (sa.sun_path, ADDRESS);

    unlink (ADDRESS);
    len = sizeof ( sa.sun_family) + strlen (sa.sun_path);
    if ( bind ( d, &sa, len) < 0 ) {
        perror ("server: bind"); exit (1);
    }
    
    pthread_t th1;
    pthread_create(&th1,NULL,ListenSocket,(void*)d);
    printf(">>listner created\n");
}

int ChangeFileEmblem(NautilusFileInfo *file, int iconIndex)
{
    switch (iconIndex)    {
    case 1:{
        nautilus_file_info_add_emblem(file, emblems[1]);
        printf("emblem: 1 \n");
        break;
    }
    case 2:{
        nautilus_file_info_add_emblem(file, emblems[2]);
        printf("emblem: 2 \n");
        break;
    }
    case 3:{
        nautilus_file_info_add_emblem(file, emblems[3]);
        printf("emblem: 3 \n");
        break;
    }
    case 4:{
        nautilus_file_info_add_emblem(file, emblems[4]);
        printf("emblem: 4 \n");
        break;
    }
    case 5:{
        nautilus_file_info_add_emblem(file, emblems[5]);
        printf("emblem: 5 \n");
        break;
    }
    case 8:{
        nautilus_file_info_add_emblem(file, emblems[8]);
        printf("emblem: 3 \n");
        break;
    }
    }
}

static NautilusOperationResult nautilus_test_update_file_info (
    NautilusInfoProvider * const info_provider,
    NautilusFileInfo * const nautilus_file,
    GClosure * const update_complete,
    NautilusOperationHandle ** const operation_handle
) {
    gchar *path;
    GFile *location = nautilus_file_info_get_location(nautilus_file);
    path = g_file_get_path(location);
    if (!path)
    {
        return NAUTILUS_OPERATION_COMPLETE;
    }

    printf(">>> File_location - %s \n",path);
    g_object_unref(location);

    int iconInd = request_state(path);
    nautilus_file_info_invalidate_extension_info(nautilus_file);

    printf("\nIcon index - %d \n", iconInd);
    ChangeFileEmblem(nautilus_file, iconInd);

    return NAUTILUS_OPERATION_COMPLETE;
}


static void nautilus_test_type_info_provider_iface_init (
    NautilusInfoProviderIface * const iface,
    gpointer const iface_data)
{
    printf("nautilus_test_type_info_provider_iface_init \n");
    iface->update_file_info = nautilus_test_update_file_info;
}


static void nautilus_test_class_init (
	NautilusTestClass * const nautilus_test_class,
    gpointer class_data)
{
    printf("nautilus_test_class_init\n");
    parent_class = g_type_class_peek_parent(nautilus_test_class);
}


static void nautilus_test_register_type (
    GTypeModule * const module)
{
    printf("nautilus_test_register_type \n");
    static const GTypeInfo info = {
        sizeof(NautilusTestClass),
        (GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) nautilus_test_class_init,
		(GClassFinalizeFunc) NULL,
		NULL,
		sizeof(NautilusTest),
		0,
		(GInstanceInitFunc) NULL,
		(GTypeValueTable *) NULL
	};

    nautilus_test_type = g_type_module_register_type(
                module,
                G_TYPE_OBJECT,
                "NautilusTest",
                &info,
                0); 

    static const GInterfaceInfo type_info_provider_iface_info = {
        (GInterfaceInitFunc) nautilus_test_type_info_provider_iface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL};

    g_type_module_add_interface(
                module,
                nautilus_test_type,
                NAUTILUS_TYPE_INFO_PROVIDER,
                &type_info_provider_iface_info);
}




GType nautilus_test_get_type (void)
{
	return nautilus_test_type;
}


void nautilus_module_shutdown (void)
{
	/*  Any module-specific shutdown  */
}


void nautilus_module_list_types (
        const GType** const types,
        int* const num_types)
{
    *types = provider_types;
    *num_types = G_N_ELEMENTS(provider_types);
}


// Extension initialization 
void nautilus_module_initialize (
    GTypeModule * const module)
{
    printf(">>nautilus_module_initialize \n");    
    I18N_INIT();
    nautilus_test_register_type(module);
    *provider_types = nautilus_test_get_type();
    FileStateListner();
}


/*  EOF  */
