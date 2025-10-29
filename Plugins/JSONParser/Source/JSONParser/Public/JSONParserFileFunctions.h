// Copyright 2023 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "JSONParser.h"
#include "Misc/InteractiveProcess.h"
#include "JSONParserFileFunctions.generated.h"



UCLASS(Blueprintable, BlueprintType)
class JSONPARSER_API UJSONParserFileFunctions : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	static UJSONParserFileFunctions* fileFunctionsJSONParser;
	UFUNCTION()
		static UJSONParserFileFunctions* getFileFunctionsJSONParser();


	static FString getCleanDirectory(EJSONDirectoryType directoryType, FString filePath);

	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void writeBytesToFile(EJSONDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void addBytesToFile(EJSONDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void splitFile(bool& success, TArray<FString>& absolutePathsOfTheParts, TArray<int64>& sizeOfTheParts, EJSONDirectoryType directoryType, FString filePath, FString additionalSubFolder, int32 parts);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static TArray<uint8> readBytesFromFile(EJSONDirectoryType directoryType, FString filePath, bool &success);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void readStringFromFile(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& data);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void writeStringToFile(EJSONDirectoryType directoryType, FString data, FString filePath, bool& success);

	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void getMD5FromFile(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& MD5);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void getMD5(const TArray<uint8>& byteArray, FString& MD5);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void bytesToBase64String(TArray<uint8> bytes, FString& base64String);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static TArray<uint8> base64StringToBytes(FString base64String, bool& success);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void stringToBase64String(FString string, FString& base64String);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void base64StringToString(FString& string, FString base64String);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void fileToBase64String(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& base64String, FString& fileName);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool fileExists(EJSONDirectoryType directoryType, FString filePath);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool directoryExists(EJSONDirectoryType directoryType, FString path);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static int64 fileSize(EJSONDirectoryType directoryType, FString filePath);
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool deleteFile(EJSONDirectoryType directoryType, FString filePath);
	/** Delete a directory and return true if the directory was deleted or otherwise does not exist. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool deleteDirectory(EJSONDirectoryType directoryType, FString filePath);
	/** Return true if the file is read only. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool isReadOnly(EJSONDirectoryType directoryType, FString filePath);
	/** Attempt to move a file. Return true if successful. Will not overwrite existing files. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool moveFile(EJSONDirectoryType directoryTypeTo, FString filePathTo, EJSONDirectoryType directoryTypeFrom, FString filePathFrom);
	/** Copy a file. This will fail if the destination file already exists. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool copyFile(EJSONDirectoryType directoryTypeTo, FString filePathTo, EJSONDirectoryType directoryTypeFrom, FString filePathFrom);
	/** Attempt to change the read only status of a file. Return true if successful. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool setReadOnly(EJSONDirectoryType directoryType, FString filePath, bool bNewReadOnlyValue);
	/** Return the modification time of a file. Returns FDateTime::MinValue() on failure **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static FDateTime getTimeStamp(EJSONDirectoryType directoryType, FString filePath);
	/** Sets the modification time of a file **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static void	setTimeStamp(EJSONDirectoryType directoryType, FString filePath, FDateTime DateTime);
	/** Return the last access time of a file. Returns FDateTime::MinValue() on failure **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static FDateTime getAccessTimeStamp(EJSONDirectoryType directoryType, FString filePath);
	/** For case insensitive filesystems, returns the full path of the file with the same case as in the filesystem */
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static FString getFilenameOnDisk(EJSONDirectoryType directoryType, FString filePath);
	/** Create a directory and return true if the directory was created or already existed. **/
	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool createDirectory(EJSONDirectoryType directoryType, FString path);


	static void LoadFile(EJSONDirectoryType directoryType, FString filePath, FString& fileName, TArray<uint8>& bytes, int32& errorCode, FString& errorMessage);
	//void LoadFileAsync(EJSONDirectoryType directoryType, FString filePath, FString eventID);
	static void SaveFile(EJSONDirectoryType directoryType, FString filePath, bool createDirectories, bool overwriteFile, TArray<uint8> bytes, FString& fileName, FString& fullPath, int32& errorCode, FString& errorMessage);
	//void SaveFileAsync(EJSONDirectoryType directoryType, FString filePath, bool createDirectories, bool overwriteFile, TArray<uint8> bytes, FString eventID);
	static bool LoadAllFilesFromDirectory(EJSONDirectoryType directoryType, FString fileType, FString dirPath, bool recursiveSearch, int32& count, TArray<FString>& files, TArray<FString>& filePaths);
	static bool LoadAllDirectorysFromDirectory(EJSONDirectoryType directoryType, FString endsWith, FString dirPath, /*bool recursiveSearch,*/ int32& count, TArray<FString>& files, TArray<FString>& filePaths);
	//void LoadAllFilesFromDirectoryAsync(EJSONDirectoryType directoryType, FString fileType, FString dirPath, bool recursiveSearch, FString eventID);

	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static FString Base64Encode(TArray<uint8> bytes);

	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static TArray<uint8> Base64Decode(FString base64EncodedString);


	UFUNCTION(BlueprintCallable, Category = "JSON|File")
		static bool base64StringToFile(EJSONDirectoryType directoryType, FString base64String, FString filePath);

	static FString Int64ToFString(int64 Num);




};

