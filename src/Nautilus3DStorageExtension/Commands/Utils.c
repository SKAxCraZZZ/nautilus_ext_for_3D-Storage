//
// Created by sheth on 29.01.24.
//

#include "Utils.h"
#include "Proto/DataContract.pb-c.h"
#include <stdlib.h>
#include <sys/un.h>
#include <nautilus-extension.h>

struct DataContractMessage SerializeDataContractMessage(DataContracts__CommandInvokeData commandInvokeData) {
	void *packed;
	int size;
	size = data_contracts__command_invoke_data__get_packed_size(&commandInvokeData);
	packed = malloc(size);
	data_contracts__command_invoke_data__pack(&commandInvokeData, packed);

	char bytes[4];
	IntToBytes(size, (char *) bytes);

	int resultStringSize;
	resultStringSize = sizeof(bytes) + size;
	unsigned char *resultString;
	resultString = malloc(resultStringSize);

	memcpy(resultString, &bytes, sizeof(bytes));
	memcpy(resultString + sizeof(bytes), packed, size);

	resultString[sizeof(bytes) + size] = '\0';

	struct DataContractMessage result;
	result.messageLength = resultStringSize;
	result.message = malloc(resultStringSize);
	result.message = resultString;

	free(packed);
	return result;
}

GList *
BuildContextMenu(NautilusMenuProvider *provider, DataContracts__MenuData dataContractsMenuData, GList *file_selection,
				 gpointer comm) {
	GList *contextMenuItemslist;
	contextMenuItemslist = NULL;
	int iterator;
	iterator = 0;
	while (iterator < dataContractsMenuData.n_items) {
		char *commandTitle = dataContractsMenuData.items[iterator]->header;
		RemoveChar(commandTitle, '&');

		NautilusMenuItem *const menu_item = nautilus_menu_item_new(
				g_strconcat("Nautilus3DStorage::menuItem_", dataContractsMenuData.items[iterator]->commandid, NULL),
				(commandTitle),
				("Nautilus3DStorage_menu_Item"),
				NULL
		);

		g_signal_connect(
				menu_item,
				"activate",
				G_CALLBACK(comm),
				provider
		);

		if (!dataContractsMenuData.items[iterator]->isenabled)
			SetSensitiveFalse(menu_item);

		g_object_set_data(G_OBJECT(menu_item), "commandIdHi",
						  (gpointer) dataContractsMenuData.items[iterator]->commandid->hi);
		g_object_set_data(G_OBJECT(menu_item), "commandIdLo",
						  (gpointer) dataContractsMenuData.items[iterator]->commandid->lo);
		g_object_set_data(G_OBJECT(menu_item), "3DStorage:files", nautilus_file_info_list_copy(file_selection));

		contextMenuItemslist = g_list_append(contextMenuItemslist, menu_item);
		iterator++;
	}

	return contextMenuItemslist;
}

void IntToBytes(int paramInt, char *bytes) {
	bytes[3] = (paramInt >> 24) & 0xFF;
	bytes[2] = (paramInt >> 16) & 0xFF;
	bytes[1] = (paramInt >> 8) & 0xFF;
	bytes[0] = paramInt & 0xFF;
}

void SetSensitiveFalse(void *object) {
	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_BOOLEAN);
	g_value_set_boolean(&value, FALSE);
	g_object_set_property(G_OBJECT(object), "sensitive", &value);
}

int BytesToInt(char *bytes) {
	int i =
			(bytes[3] << 24) |
			(bytes[2] << 16) |
			(bytes[1] << 8) |
			(bytes[0]);

	return i;
}

void RemoveChar(char *inputString, char removedChar) {
	printf("inputString: %s \n, strlen - %i \n", inputString, strlen(inputString));
	int removedCharPosition = strcspn(inputString, &removedChar);
	if (removedCharPosition < strlen(inputString))
		memmove(inputString + removedCharPosition, inputString + removedCharPosition + 1,
				strlen(inputString) - removedCharPosition);
}