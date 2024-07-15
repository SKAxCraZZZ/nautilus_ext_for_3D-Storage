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
#include <pthread.h>
#include <locale.h>
#include "Commands/Proto/protobuf-c/protobuf-c.h"
#include "Commands/Proto/DataContract.pb-c.h"
#include "Commands/Utils.h"

#define FILE_URI_PREFIX "file://"
#define AF_UNIX 1
#define ADDRESS_FOR_NOTIF_STATE "/tmp/CoreFxPipe_IconStateNotifierPipe"
#define ADDRESS_FOR_REQ_STATE "/tmp/CoreFxPipe_PilotInfoPipepfmfs.3D-Storage20170614"
#define ADDRESS_FOR_COMMAND "/tmp/CoreFxPipe_PilotCommandPipepfmfs.3D-Storage20170614"
#define RECV_MESSAGE_LENGTH 1000

/*\
|*|
|*| BUILD SETTINGS
|*|
\*/

#ifdef ENABLE_NLS
#include <glib/gi18n-lib.h>
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

int currentEmblem = 0;
static char* emblems[] = {
		"", "emblem-loadedstore", "emblem-outdatedstore", "emblem-notsentstore", "emblem-editedstore",
		"emblem-abortedstore", "", "",
		"emblem-conflictstore"
};

static GType provider_types[1];
static GType nautilus_3dstorage_extension_type;
static GObjectClass* parent_class;

typedef struct FileStateInfo {
	char* uri;
	int icon_index;
} FileStateInfo;

/*\
|*|
|*| FUNCTIONS
|*|
\*/
char* ReadString(FILE* file);

char* PrependStringLength(char* string);

char* GetFilePath(NautilusFileInfo fileInfo);

FileStateInfo GetFileInfo(FILE* pipe_file);

char* AddFilePrefix(char* path_to_file);

DataContracts__CommandInvokeResult SerializeAndSend(DataContracts__CommandInvokeData instance);

static void commandInvoke(NautilusMenuItem* item);

int ChangeFileEmblem(NautilusFileInfo* file, int icon_index);

char* ReadString(FILE* file) {
	int lenght, i;
	int char_count = 256;
	unsigned char byte[2];

	byte[0] = fgetc(file);
	byte[1] = fgetc(file);

	lenght = byte[0] * char_count + byte[1];
	unsigned char bytes[lenght];

	i = 0;
	while (i != lenght) {
		bytes[i] = fgetc(file);
		i++;
	}

	char* chars = (char*)malloc(lenght + 1);
	memcpy(chars, bytes, lenght);
	chars[lenght] = '\0';
	return chars;
}

char* PrependStringLength(char* string) {
	int stringLength = strlen(string);
	char stringLengthFirstByte = (char)(stringLength / 255);
	char stringLengthSecondByte = (char)(stringLength % 255);

	char* resultString = (char*)malloc(sizeof(char) + sizeof(char) + stringLength);
	memcpy(resultString, &stringLengthFirstByte, sizeof(char));
	memcpy(resultString + sizeof(char), &stringLengthSecondByte, sizeof(char));
	memcpy(resultString + sizeof(char) + sizeof(char), string, stringLength);
	return resultString;
}

FileStateInfo GetFileInfo(FILE* pipe_file) {
	int b;
	FileStateInfo file_info;
	for (b = 0; b < 2; b++) {
		char* chars;
		chars = ReadString(pipe_file);

		if (strlen(chars) > 2)
			file_info.uri = AddFilePrefix(chars);
		else
			file_info.icon_index = atoi(chars);
	}
	printf(">>> result uri - %s \n", file_info.uri);
	printf(">>> result index - %i \n", file_info.icon_index);
	return file_info;
}

char* AddFilePrefix(char* path_to_file) {
	char* uri = malloc(strlen(FILE_URI_PREFIX) + strlen(path_to_file) + 1);
	memcpy(uri, FILE_URI_PREFIX, strlen(FILE_URI_PREFIX));
	memcpy(uri + strlen(FILE_URI_PREFIX), path_to_file, strlen(path_to_file) + 1);
	return uri;
}

static int RequestState(char* path) {
	int socket_for_requests = 0;
	int data_len = 0;
	struct sockaddr_un socket_address;

	char* PathWithStringLength = PrependStringLength(path);
	int sizeOfTwoByteForStrLength = sizeof(char) * 2;
	int sizeOfTerminalNull = 2;
	int sendMessageLength = strlen(PathWithStringLength + sizeOfTwoByteForStrLength) + sizeOfTerminalNull;
	char recv_msg[RECV_MESSAGE_LENGTH];

	memset(recv_msg, 0, RECV_MESSAGE_LENGTH);

	if ((socket_for_requests = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		printf("Client: Error on socket() call \n");
	}

	socket_address.sun_family = AF_UNIX;
	strcpy(socket_address.sun_path, ADDRESS_FOR_REQ_STATE);
	data_len = strlen(socket_address.sun_path) + sizeof(socket_address.sun_family);

	printf("Client: Trying to connect... \n");
	if (connect(socket_for_requests, (struct sockaddr*)&socket_address, data_len) == -1) {
		printf("Client: Error on connect call \n");
	}
	printf("Client: Connected \n");

	if (send(socket_for_requests, PathWithStringLength, sendMessageLength, 0) == -1) {
		printf("Client: Error on send() call2 \n");
	}

	free(PathWithStringLength);
	if ((data_len = recv(socket_for_requests, recv_msg, RECV_MESSAGE_LENGTH, 0)) > 0) {
		printf("Client: Data received upd: %s \n", recv_msg);
		close(socket_for_requests);
		return atoi(recv_msg);
	}
	else {
		if (data_len < 0) {
			printf("Client: Error on recv() call \n");
		}
		else {
			printf("Client: Server socket closed \n");
			close(socket_for_requests);
		}
		return 0;
	}
}

void* ListenSocket(void* arg) {
	struct sockaddr_un socket_address;
	int address_length, file_descriptor;
	FILE* pipe_file;
	char* uri;
	int icon_index;
	FileStateInfo file_info;

	while (TRUE) {
		if (listen((uintptr_t)arg, 5) < 0) {
			perror("server: listen");
			exit(1);
			printf(">>exit\n");
		}
		printf(">>connect to client\n");
		if ((file_descriptor = accept((uintptr_t)arg, (struct sockaddr*)&socket_address, &address_length)) < 0) {
			perror("server: accept");
			exit(1);
		}
		/* ------------------------------------------ */
		printf(">>reading\n");

		pipe_file = fdopen(file_descriptor, "r");
		file_info = GetFileInfo(pipe_file);

		printf(">>> result uri - %s \n", file_info.uri);
		printf(">>> result index - %i \n", file_info.icon_index);

		NautilusFileInfo* file = nautilus_file_info_lookup_for_uri(file_info.uri);
		if (file != NULL) {
			nautilus_file_info_invalidate_extension_info(file);
			ChangeFileEmblem(file, file_info.icon_index);
		}
	}
}

void FileStateListener() {
	void* state_listener_socket;
	int length;
	struct sockaddr_un socket_address;

	printf(">>get socket descriptor\n");
	state_listener_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	if (state_listener_socket < 0) {
		perror("client: socket");
		exit(1);
	}

	socket_address.sun_family = AF_UNIX;
	strcpy(socket_address.sun_path, ADDRESS_FOR_NOTIF_STATE);

	unlink(ADDRESS_FOR_NOTIF_STATE);
	length = sizeof(socket_address.sun_family) + strlen(socket_address.sun_path);
	if (bind((uintptr_t)state_listener_socket, (struct sockaddr*)&socket_address, length) < 0) {
		perror("server: bind");
		exit(1);
	}
	printf(">>listening to requests\n");
	pthread_t th1;
	pthread_create(&th1, NULL, ListenSocket, state_listener_socket);
	printf(">>listener created\n");
}

int ChangeFileEmblem(NautilusFileInfo* file, int icon_index) {
	switch (icon_index) {
	case 0: {
		nautilus_file_info_add_emblem(file, emblems[0]);
		printf("emblem: 0 \n");
		break;
	}
	case 1: {
		nautilus_file_info_add_emblem(file, emblems[1]);
		printf("emblem: 1 \n");
		break;
	}
	case 2: {
		nautilus_file_info_add_emblem(file, emblems[2]);
		printf("emblem: 2 \n");
		break;
	}
	case 3: {
		nautilus_file_info_add_emblem(file, emblems[3]);
		printf("emblem: 3 \n");
		break;
	}
	case 4: {
		nautilus_file_info_add_emblem(file, emblems[4]);
		printf("emblem: 4 \n");
		break;
	}
	case 5: {
		nautilus_file_info_add_emblem(file, emblems[5]);
		printf("emblem: 5 \n");
		break;
	}
	case 8: {
		nautilus_file_info_add_emblem(file, emblems[8]);
		printf("emblem: 3 \n");
		break;
	}
	}
	return 0;
}

static NautilusOperationResult nautilus_3dstorage_extension_update_file_info(
	NautilusInfoProvider* const info_provider,
	NautilusFileInfo* const nautilus_file,
	GClosure* const update_complete,
	NautilusOperationHandle** const operation_handle
) {
	char* path;
	setlocale(LC_ALL, "");
	GFile* location = nautilus_file_info_get_location(nautilus_file);
	path = g_file_get_path(location);
	if (!path) {
		return NAUTILUS_OPERATION_COMPLETE;
	}
	printf(">>> File_location - %s \n", path);
	g_object_unref(location);

	int iconInd = RequestState(path);

	nautilus_file_info_invalidate_extension_info(nautilus_file);

	printf("\nIcon index - %d \n", iconInd);
	ChangeFileEmblem(nautilus_file, iconInd);

	return NAUTILUS_OPERATION_COMPLETE;
}


static void nautilus_3dstorage_extension_type_info_provider_iface_init(
	NautilusInfoProviderIface* const iface) {
	iface->update_file_info = nautilus_3dstorage_extension_update_file_info;
}

static void nautilus_3dstorage_extension_class_init(
	GObjectClass* const nautilus_3dstorage_extension_class) {
	parent_class = g_type_class_peek_parent(nautilus_3dstorage_extension_class);
}

static GList* nautilus_3dstorage_extension_get_file_items(NautilusMenuProvider* provider,
	G_GNUC_UNUSED GtkWidget* window,
	GList* file_selection) {
	printf("nautilus_3dstorage_extension_get_file_items init\n");
	if (!file_selection) {
		printf("return NULL\n");
		return NULL;
	}
	DataContracts__CommandInvokeData ContextMenuRequestCommand = DATA_CONTRACTS__COMMAND_INVOKE_DATA__INIT;
	DataContracts__Guid ContextMenuRequestCommandGuid = DATA_CONTRACTS__GUID__INIT;

	ContextMenuRequestCommandGuid.lo = 5059794805538720836;
	ContextMenuRequestCommandGuid.hi = 5424190233400613762;
	ContextMenuRequestCommand.commandid = &ContextMenuRequestCommandGuid;
	ContextMenuRequestCommand.commandid->has_hi = 1;
	ContextMenuRequestCommand.commandid->has_lo = 1;

	int selectedFilesCount = g_list_length(file_selection);
	int count = 0;
	char** pathCommand;

	printf("Converted selected files count - %i \n", selectedFilesCount);
	GList* l;

	pathCommand = malloc(selectedFilesCount * sizeof(char*));
	if (pathCommand != NULL)
		for (l = file_selection; l != NULL; l = l->next) {
			NautilusFileInfo* file = NAUTILUS_FILE_INFO(l->data);
			gchar* path;
			GFile* fp;
			fp = nautilus_file_info_get_location(file);
			if (!fp) {
				continue;
			}
			path = g_file_get_path(fp);
			if (!path) {
				continue;
			}

			pathCommand[count] = calloc(strlen(path) + 1, sizeof(char));
			if (pathCommand[count] != NULL) {
				memcpy(pathCommand[count], path, strlen(path) * sizeof(char));
			}
			count++;
			free(path);
		}

	printf("Converted path - ready\n");
	ContextMenuRequestCommand.n_paths = selectedFilesCount;
	ContextMenuRequestCommand.paths = pathCommand;

	DataContracts__CommandInvokeResult commandInvokeResult;
	commandInvokeResult = SerializeAndSend(ContextMenuRequestCommand);
	GList* contextMenuItemslist;
	contextMenuItemslist = NULL;

	printf("ContextMenuRequestCommandResult is OK - %i\n",
		commandInvokeResult.result == DATA_CONTRACTS__SHELL_RESULT__Ok);
	printf("ContextMenuRequestCommandResult - HasData %i\n", commandInvokeResult.has_data == TRUE);
	if (commandInvokeResult.result == DATA_CONTRACTS__SHELL_RESULT__Ok)
		if (commandInvokeResult.has_data == TRUE) {
			DataContracts__MenuData menu_data;
			menu_data = *data_contracts__menu_data__unpack(NULL, commandInvokeResult.data.len, commandInvokeResult.data.data);
			printf("Client: Data received menu count %li\n", menu_data.n_items);

			if (menu_data.n_items > 0) {
				contextMenuItemslist = BuildContextMenu(provider, menu_data, file_selection, commandInvoke);
			}
		}
	for (int i = 0; i < selectedFilesCount; i++)
		free(pathCommand[i]);
	free(pathCommand);
	return contextMenuItemslist;
}

static void commandInvoke(NautilusMenuItem* item) {
	GList* l;
	GList* files = g_object_get_data(G_OBJECT(item), "3DStorage:files");
	int count = 0;

	char** pathCommand = (char**)malloc(g_list_length(files) * sizeof(char*));
	for (l = files; l != NULL; l = l->next) {
		NautilusFileInfo* file = NAUTILUS_FILE_INFO(l->data);
		char* path;
		GFile* fp;
		fp = nautilus_file_info_get_location(file);
		if (!fp) {
			continue;
		}

		path = g_file_get_path(fp);
		if (!path) {
			continue;
		}

		pathCommand[count] = calloc(strlen(path) + 1, sizeof(char));
		if (pathCommand[count] != NULL) {
			memcpy(pathCommand[count], path, strlen(path));
		}
		count++;
		free(path);
	}

	DataContracts__CommandInvokeData dataContractsCommandData = DATA_CONTRACTS__COMMAND_INVOKE_DATA__INIT;
	DataContracts__Guid guid = DATA_CONTRACTS__GUID__INIT;

	guid.hi = (long)g_object_get_data(G_OBJECT(item), "commandIdHi");
	guid.lo = (long)g_object_get_data(G_OBJECT(item), "commandIdLo");

	dataContractsCommandData.commandid = &guid;
	dataContractsCommandData.commandid->has_hi = 1;
	dataContractsCommandData.commandid->has_lo = 1;

	dataContractsCommandData.paths = pathCommand;
	dataContractsCommandData.n_paths = g_list_length(files);

	printf(">>> commandInvoke hi - %lu\n", dataContractsCommandData.commandid->hi);
	printf(">>> commandInvoke lo - %lu\n", dataContractsCommandData.commandid->lo);
	SerializeAndSend(dataContractsCommandData);
	printf(">>> commandInvoke - done\n");
	for (int i = 0; i < g_list_length(files); i++)
		free(pathCommand[i]);
	free(pathCommand);
}


static void nautilus_3dstorage_extension_menu_provider_iface_init(
	NautilusMenuProviderInterface* const iface) {
	printf(">>> nautilus_3dstorage_extension_menu_provider_iface_init - done\n");
	iface->get_file_items = nautilus_3dstorage_extension_get_file_items;
	printf(">>> nautilus_3dstorage_extension_menu_provider_iface_init - done\n");
}


static void nautilus_register_types(
	GTypeModule* const module) {
	static const GTypeInfo info = {
			sizeof(GObjectClass),
			(GBaseInitFunc)NULL,
			(GBaseFinalizeFunc)NULL,
			(GClassInitFunc)nautilus_3dstorage_extension_class_init,
			(GClassFinalizeFunc)NULL,
			NULL,
			sizeof(GObject),
			0,
			(GInstanceInitFunc)NULL,
			(GTypeValueTable*)NULL
	};

	static const GInterfaceInfo type_info_provider_iface_info = {
			(GInterfaceInitFunc)nautilus_3dstorage_extension_type_info_provider_iface_init,
			(GInterfaceFinalizeFunc)NULL,
			NULL
	};

	static const GInterfaceInfo menu_provider_iface_info = {
			(GInterfaceInitFunc)nautilus_3dstorage_extension_menu_provider_iface_init,
			(GInterfaceFinalizeFunc)NULL,
			NULL
	};

	nautilus_3dstorage_extension_type = g_type_module_register_type(
		module,
		G_TYPE_OBJECT,
		"Nautilus3DStorageExtension",
		&info,
		0);

	g_type_module_add_interface(
		module,
		nautilus_3dstorage_extension_type,
		NAUTILUS_TYPE_MENU_PROVIDER,
		&menu_provider_iface_info);

	g_type_module_add_interface(
		module,
		nautilus_3dstorage_extension_type,
		NAUTILUS_TYPE_INFO_PROVIDER,
		&type_info_provider_iface_info);
}


GType nautilus_3dstorage_extension_get_type(void) {
	return nautilus_3dstorage_extension_type;
}


void nautilus_module_shutdown(void) {
	/*  Any module-specific shutdown  */
}


void nautilus_module_list_types(
	const GType** const types,
	int* const num_types) {
	*types = provider_types;
	*num_types = G_N_ELEMENTS(provider_types);
}


// Extension initialization
void nautilus_module_initialize(
	GTypeModule* const module) {
	printf(">>nautilus_module_initialize \n");
	nautilus_register_types(module);
	*provider_types = nautilus_3dstorage_extension_get_type();
	FileStateListener();
}

DataContracts__CommandInvokeResult SerializeAndSend(DataContracts__CommandInvokeData instance) {
	int socket_for_requests = 0;
	int data_len = 0;
	struct sockaddr_un socket_address;
	struct DataContractMessage serializedDataContractMessage;
	char* recv_msg;
	DataContracts__CommandInvokeResult res;
	res.result = DATA_CONTRACTS__SHELL_RESULT__Error;
	res.has_result = 1;
	res.has_data = 0;

	printf("\nIcon index test DataContract - start \n");
	serializedDataContractMessage = SerializeDataContractMessage(instance);

	recv_msg = malloc(4 * sizeof(char));
	if ((socket_for_requests = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		printf("Client: Error on socket() call \n");
	}
	socket_address.sun_family = AF_UNIX;
	strcpy(socket_address.sun_path, ADDRESS_FOR_COMMAND);
	data_len = strlen(socket_address.sun_path) + sizeof(socket_address.sun_family);

	if (connect(socket_for_requests, (struct sockaddr*)&socket_address, data_len) == -1) {
		printf("Client: Error on connect call \n");
	}

	printf("Client: Connected \n");

	if (send(socket_for_requests, serializedDataContractMessage.message, serializedDataContractMessage.messageLength,
		0) == -1) {
		printf("Client: Error on send() size \n");
	}

	int sizeResMessage = 0;
	if ((data_len = recv(socket_for_requests, recv_msg, 4, 0)) > 0) {
		sizeResMessage = BytesToInt((char*)recv_msg);
		printf("Client: Data received upd size: %i \n", sizeResMessage);
		recv_msg = malloc(sizeResMessage * sizeof(char));
	}
	if ((data_len = recv(socket_for_requests, recv_msg, sizeResMessage, 0)) > 0) {
		printf("Client: Data received data_len: %i \n", data_len);
		res = *data_contracts__command_invoke_result__unpack(NULL, data_len, recv_msg);
		printf("Client: Data received data_len: %i \n", data_len);
		close(socket_for_requests);
	}
	else {
		if (data_len < 0) {
			printf("Client: Error on recv() call \n");
		}
		else {
			printf("Client: Server socket closed \n");
			close(socket_for_requests);
		}
	}
	free(recv_msg);
	return res;
}

/*  EOF  */
