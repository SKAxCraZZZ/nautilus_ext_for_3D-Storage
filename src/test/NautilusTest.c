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

#define FILE_URI_PREFIX "file://"
#define AF_UNIX 1
#define ADDRESS_FOR_NOTIF_STATE "/tmp/CoreFxPipe_IconStateNotifierPipe"
#define ADDRESS_FOR_REQ_STATE "/tmp/CoreFxPipe_PilotInfoPipeshethpfmfs.3D-Storage20170614"
#define RECV_MESSAGE_LENGH 5
#define SEND_MESSAGE_LENGH 250

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
} Nautilus3DStorageExtension;

typedef struct {
	GObjectClass parent_slot;
} Nautilus3DStorageExtensionClass;

static GType provider_types[1];
static GType nautilus_3dstorage_extension_type;
static GObjectClass *parent_class;

/*\
|*|
|*| FUNCTIONS
|*|
\*/

char* ReadString(FILE *file);
int ChangeFileEmblem(NautilusFileInfo *file, int iconIndex);

char* ReadString (FILE *file)
{
    int lenght,i;
    unsigned char byte[2];

    byte[0]=fgetc(file);
    byte[1]=fgetc(file);

    lenght= byte[0] * 256 + byte[1];
    unsigned char bytes[lenght];

    i=0;
    while (i != lenght)
    {
        bytes[i] = fgetc(file);
        i++;
    }

    char* chars= (char*)malloc(lenght);
    memcpy(chars, bytes, lenght);
    chars[lenght] = '\0';
    return chars;
}

static int Request_state(char *path)
{
    int sock = 0;
    int data_len = 0;
    struct sockaddr_un remote;    

    char recv_msg[RECV_MESSAGE_LENGH];
    char send_msg[SEND_MESSAGE_LENGH];

    memset(recv_msg, 0, RECV_MESSAGE_LENGH * sizeof(char));
    memset(send_msg, 0, SEND_MESSAGE_LENGH * sizeof(char));

    if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        printf("Client: Error on socket() call \n");
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, ADDRESS_FOR_REQ_STATE );
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
        printf("Client: Error on send() call2 \n");
    }
    memset(send_msg, 0, SEND_MESSAGE_LENGH * sizeof(char));
    memset(recv_msg, 0, RECV_MESSAGE_LENGH * sizeof(char));

    if ((data_len = recv(sock, recv_msg, RECV_MESSAGE_LENGH, 0)) > 0)
    {
        printf("Client: Data received upd: %s \n", recv_msg);        
        close(sock);
        return atoi(recv_msg);
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
    int ca_len,d1,i;
    FILE* pipefile;
    int d = (int)arg;

    char* uri;
    int iconindex;

    while(TRUE)
    {
        if ( listen ( d, 5) < 0 )
        {
            perror ("server: listen"); exit (1);
            printf(">>exit\n");
        }
        printf(">>связываемся с клиентом\n");
        if (( d1 = accept ( d, &ca, &ca_len)) < 0 )
        {
            perror ("server: accept"); exit (1);
        }
        /* ------------------------------------------ */
        printf(">>reading\n");

        pipefile = fdopen (d1, "r");
        int b;
        for (b = 0; b < 2; b++)
        {
            char* chars;
            chars = ReadString(pipefile);

            if (strlen(chars)>2)
            {
                uri = malloc(strlen(FILE_URI_PREFIX)+strlen(chars)+1);
                memcpy(uri, FILE_URI_PREFIX, strlen(FILE_URI_PREFIX));
                memcpy(uri+strlen(FILE_URI_PREFIX), chars, strlen(chars)+1);
            }
            else
            {
                printf(">>>index - %s \n", chars);
                iconindex = atoi(chars);
            }
        }

        printf(">>> result uri - %s \n", uri);
        printf(">>> result index - %i \n", iconindex);

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
    int d,len;
    struct sockaddr_un sa;

    printf(">>получаем свой сокет-дескриптор \n");
    if((d = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
     perror ("client: socket"); exit (1);
    }

    sa.sun_family = AF_UNIX;
    strcpy (sa.sun_path, ADDRESS_FOR_NOTIF_STATE);

    unlink (ADDRESS_FOR_NOTIF_STATE);
    len = sizeof ( sa.sun_family) + strlen (sa.sun_path);
    if ( bind ( d, &sa, len) < 0 ) {
        perror ("server: bind"); exit (1);
    }
    printf(">>слушаем запросы\n");
    pthread_t th1;
    pthread_create(&th1,NULL,ListenSocket,(void*)d);
    printf(">>listner created\n");
}

int ChangeFileEmblem(NautilusFileInfo *file, int iconIndex)
{
    switch (iconIndex)    {
    case 7:{
        nautilus_file_info_add_emblem(file, emblems[2]);
        printf("emblem: 1 \n");
        break;
    }
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

static NautilusOperationResult nautilus_3dstorage_extension_update_file_info (
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

    int iconInd = Request_state(path);
    nautilus_file_info_invalidate_extension_info(nautilus_file);

    printf("\nIcon index - %d \n", iconInd);
    ChangeFileEmblem(nautilus_file, iconInd);

    return NAUTILUS_OPERATION_COMPLETE;
}


static void nautilus_3dstorage_extension_type_info_provider_iface_init (
    NautilusInfoProviderIface * const iface,
    gpointer const iface_data)
{
    printf("nautilus_3dstorage_extension_type_info_provider_iface_init \n");
    iface->update_file_info = nautilus_3dstorage_extension_update_file_info;
}


static void nautilus_3dstorage_extension_class_init (
    Nautilus3DStorageExtensionClass * const nautilus_3dstorage_extension_class,
    gpointer class_data)
{
    printf("nautilus_3dstorage_extension_class_init\n");
    parent_class = g_type_class_peek_parent(nautilus_3dstorage_extension_class);
}


static void nautilus_register_types (
    GTypeModule * const module)
{
    printf("nautilus_register_types \n");
    static const GTypeInfo info = {
        sizeof(Nautilus3DStorageExtensionClass),
        (GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
        (GClassInitFunc) nautilus_3dstorage_extension_class_init,
		(GClassFinalizeFunc) NULL,
		NULL,
        sizeof(Nautilus3DStorageExtension),
		0,
		(GInstanceInitFunc) NULL,
		(GTypeValueTable *) NULL
	};

    nautilus_3dstorage_extension_type = g_type_module_register_type(
                module,
                G_TYPE_OBJECT,
                "Nautilus3DStorageExtension",
                &info,
                0); 

    static const GInterfaceInfo type_info_provider_iface_info = {
        (GInterfaceInitFunc) nautilus_3dstorage_extension_type_info_provider_iface_init,
        (GInterfaceFinalizeFunc) NULL,
        NULL};

    g_type_module_add_interface(
                module,
                nautilus_3dstorage_extension_type,
                NAUTILUS_TYPE_INFO_PROVIDER,
                &type_info_provider_iface_info);
}




GType nautilus_3dstorage_extension_get_type (void)
{
    return nautilus_3dstorage_extension_type;
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
    nautilus_register_types(module);
    *provider_types = nautilus_3dstorage_extension_get_type();
    FileStateListner();
}


/*  EOF  */
