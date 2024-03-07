//
// Created by sheth on 29.01.24.
//

#include <libnautilus-extension/nautilus-menu-provider.h>
#include "Proto/protobuf-c/protobuf-c.h"
#include "Proto/DataContract.pb-c.h"

#ifndef NAUTILUS_TEST_UTILS_H
#define NAUTILUS_TEST_UTILS_H

struct DataContractMessage {
    int messageLength;
    unsigned char* message;
};

struct DataContractMessage SerializeDataContractMessage(DataContracts__CommandInvokeData commandInvokeData);
GList* BuildContextMenu(NautilusMenuProvider* provider, DataContracts__MenuData dataContractsMenuData, GList* file_selection, gpointer comm);
DataContracts__CommandInvokeResult DeserializeDataContractMessage(struct DataContractMessage);
void SetSensitiveFalse(void* object);
void IntToBytes(int paramInt, unsigned char* bytes);
int BytesToInt(unsigned char* bytes);
void RemoveChar(char* inputString, char removedChar);

#endif //NAUTILUS_TEST_UTILS_H
