// Copyright 2023 David Romanski (Socke). All Rights Reserved.

#include "JSONParserFileFunctions.h"

UJSONParserFileFunctions* UJSONParserFileFunctions::fileFunctionsJSONParser;
UJSONParserFileFunctions::UJSONParserFileFunctions(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	fileFunctionsJSONParser = this;
}

UJSONParserFileFunctions* UJSONParserFileFunctions::getFileFunctionsJSONParser() {
	return fileFunctionsJSONParser;
}


FString UJSONParserFileFunctions::getCleanDirectory(EJSONDirectoryType directoryType, FString filePath) {
	if (directoryType == EJSONDirectoryType::E_ad) {
		return FPaths::ConvertRelativePathToFull(filePath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		return FPaths::ConvertRelativePathToFull(ProjectDir + filePath);
	}
}

void UJSONParserFileFunctions::writeBytesToFile(EJSONDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	success = FFileHelper::SaveArrayToFile(bytes, *getCleanDirectory(directoryType, filePath));
}

void UJSONParserFileFunctions::addBytesToFile(EJSONDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	FArchive* writer = IFileManager::Get().CreateFileWriter(*getCleanDirectory(directoryType, filePath), EFileWrite::FILEWRITE_Append);
	if (!writer) {
		success = false;
		return;
	}
	writer->Seek(writer->TotalSize());
	writer->Serialize(bytes.GetData(), bytes.Num());
	writer->Close();
	delete writer;

	success = true;
}

void UJSONParserFileFunctions::splitFile(bool& success, TArray<FString>& absolutePathsOfTheParts, TArray<int64>& sizeOfTheParts,
	EJSONDirectoryType directoryType, FString filePath,FString additionalSubFolder, int32 parts){

	if (parts <= 0)
		parts = 1;



	FString originalFilePath = getCleanDirectory(directoryType, filePath);
	FString originalFileName = FPaths::GetCleanFilename(originalFilePath);
	FString targetFilePath = FPaths::GetPath(originalFilePath);
	if (additionalSubFolder.IsEmpty() == false) {
		targetFilePath += + "/" + additionalSubFolder;
		if (FPaths::DirectoryExists(targetFilePath) == false) {
			UJSONParserFileFunctions::createDirectory(EJSONDirectoryType::E_ad, targetFilePath);
		}

		//UE_LOG(LogTemp, Warning, TEXT("splitFile targetFilePath: %s"), *targetFilePath);
	}
	//FString originalFileNameWithoutExtension = FPaths::GetBaseFilename(originalFilePath);
	//FString extension = FPaths::GetExtension(originalFilePath);

	FArchive* reader = IFileManager::Get().CreateFileReader(*originalFilePath);
	if (!reader) {
		success = false;
		return;
	}

	if (reader->TotalSize() <= ((int64)parts)) {
		success = false;

		reader->Close();
		delete reader;
		return;
	}

	int64 partSize = reader->TotalSize()/ ((int64)parts);
	int64 savedBytes = 0;
	TArray<uint8> bytes;

	int32 iterator = 0;
	while (iterator < parts-1){
		reader->Seek(savedBytes);
		bytes.AddUninitialized(partSize);
		reader->Serialize(bytes.GetData(), partSize);
		FString newFileName = originalFileName + "_part" + FString::FromInt(iterator);
			//originalFileName.Replace(*originalFileNameWithoutExtension, *FString(originalFileNameWithoutExtension+"_"+FString::FromInt(iterator)));
		FString path = targetFilePath + "/" + newFileName;

		//UE_LOG(LogTemp, Warning, TEXT("splitFile path: %s"), *path);

		absolutePathsOfTheParts.Add(path);
		sizeOfTheParts.Add(partSize);

		if (FFileHelper::SaveArrayToFile(bytes, *path) == false) {
			success = false;
			return;
		}	
		bytes.Empty();
		savedBytes += partSize;
		iterator++;
	}
	//last part
	reader->Seek(savedBytes);
	bytes.AddUninitialized(reader->TotalSize() - savedBytes);
	reader->Serialize(bytes.GetData(), reader->TotalSize() - savedBytes);
	FString newFileName = originalFileName + "_part" + FString::FromInt(iterator);
		//originalFileName.Replace(*originalFileNameWithoutExtension, *FString(originalFileNameWithoutExtension+"_"+FString::FromInt(iterator)));
	FString path = targetFilePath + "/" + newFileName;
	absolutePathsOfTheParts.Add(path);
	if (FFileHelper::SaveArrayToFile(bytes, *path) == false) {
		success = false;
		return;
	}
	bytes.Empty();

	reader->Close();
	delete reader;
	success = true;
}

TArray<uint8> UJSONParserFileFunctions::readBytesFromFile(EJSONDirectoryType directoryType, FString filePath, bool& success) {
	TArray<uint8> result;
	success = FFileHelper::LoadFileToArray(result, *getCleanDirectory(directoryType, filePath));
	return result;
}

void UJSONParserFileFunctions::readStringFromFile(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& data) {
	data.Empty();
	success = FFileHelper::LoadFileToString(data, *getCleanDirectory(directoryType, filePath));
}

void UJSONParserFileFunctions::writeStringToFile(EJSONDirectoryType directoryType, FString data, FString filePath, bool& success) {
	success = FFileHelper::SaveStringToFile(data, *getCleanDirectory(directoryType, filePath));
}


void UJSONParserFileFunctions::getMD5FromFile(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& MD5) {
	MD5.Empty();

	FString dir = getCleanDirectory(directoryType, filePath);
	FMD5Hash md5Hash = FMD5Hash::HashFile(*dir);
	if (md5Hash.IsValid() == false) {
		success = false;
		return;
	}
	TArray<uint8> byteArrayTmp;
	byteArrayTmp.Append(md5Hash.GetBytes(),md5Hash.GetSize());
	for (int32 i = 0; i < byteArrayTmp.Num(); i++) {
		MD5 += FString::Printf(TEXT("%02x"), byteArrayTmp[i]);
	}

	success = true;
}

void UJSONParserFileFunctions::getMD5(const TArray<uint8>& byteArray, FString& MD5){
	FMD5 Md5Gen;
	Md5Gen.Update(byteArray.GetData(), byteArray.Num());

	uint8 Digest[16]; // MD5 128 Bit = 16 Bytes
	Md5Gen.Final(Digest);

	FMD5Hash Hash;
	Hash.Set(Md5Gen);

	MD5.Empty();
	for (int32 i = 0; i < 16; i++) {
		MD5 += FString::Printf(TEXT("%02x"), Digest[i]);
	}
}



void UJSONParserFileFunctions::bytesToBase64String(TArray<uint8> bytes, FString& base64String) {
	base64String.Empty();
	base64String = FBase64::Encode(bytes);
}

TArray<uint8> UJSONParserFileFunctions::base64StringToBytes(FString base64String, bool& success) {
	TArray<uint8> fileData;
	success = FBase64::Decode(*base64String, fileData);
	return fileData;
}

void UJSONParserFileFunctions::stringToBase64String(FString string, FString& base64String) {
	base64String.Empty();
	FTCHARToUTF8 Convert(*string);
	TArray<uint8> bytes;
	bytes.Append((uint8*)Convert.Get(), Convert.Length());
	base64String = FBase64::Encode(bytes);
}

void UJSONParserFileFunctions::base64StringToString(FString& string, FString base64String) {
	string.Empty();
	TArray<uint8> bytes;
	if (FBase64::Decode(*base64String, bytes)) {
		bytes.Add(0x00);// null-terminator
		char* Data = (char*)bytes.GetData();
		string = FString(UTF8_TO_TCHAR(Data));
	}
}

void UJSONParserFileFunctions::fileToBase64String(EJSONDirectoryType directoryType, FString filePath, bool& success, FString& base64String, FString& fileName) {
	base64String.Empty();
	fileName.Empty();
	TArray<uint8> fileData;
	if (!FFileHelper::LoadFileToArray(fileData, *getCleanDirectory(directoryType, filePath))) {
		success = false;
		return;
	}
	base64String = FBase64::Encode(fileData);
	success = true;
}

bool UJSONParserFileFunctions::fileExists(EJSONDirectoryType directoryType, FString filePath) {
	return FPaths::FileExists(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::directoryExists(EJSONDirectoryType directoryType, FString path) {
	return FPaths::DirectoryExists(*getCleanDirectory(directoryType, path));
}

int64 UJSONParserFileFunctions::fileSize(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().FileSize(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::deleteFile(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::deleteDirectory(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteDirectoryRecursively(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::isReadOnly(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().IsReadOnly(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::moveFile(EJSONDirectoryType directoryTypeTo, FString filePathTo, EJSONDirectoryType directoryTypeFrom, FString filePathFrom) {
	return FPlatformFileManager::Get().GetPlatformFile().MoveFile(*getCleanDirectory(directoryTypeTo, filePathTo), *getCleanDirectory(directoryTypeFrom, filePathFrom));
}

bool UJSONParserFileFunctions::copyFile(EJSONDirectoryType directoryTypeTo, FString filePathTo, EJSONDirectoryType directoryTypeFrom, FString filePathFrom) {
	return FPlatformFileManager::Get().GetPlatformFile().CopyFile(*getCleanDirectory(directoryTypeTo, filePathTo), *getCleanDirectory(directoryTypeFrom, filePathFrom));
}


bool UJSONParserFileFunctions::setReadOnly(EJSONDirectoryType directoryType, FString filePath, bool bNewReadOnlyValue) {
	return FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*getCleanDirectory(directoryType, filePath), bNewReadOnlyValue);
}

FDateTime UJSONParserFileFunctions::getTimeStamp(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*getCleanDirectory(directoryType, filePath));
}

void UJSONParserFileFunctions::setTimeStamp(EJSONDirectoryType directoryType, FString filePath, FDateTime DateTime) {
	FPlatformFileManager::Get().GetPlatformFile().SetTimeStamp(*getCleanDirectory(directoryType, filePath), DateTime);
}

FDateTime UJSONParserFileFunctions::getAccessTimeStamp(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetAccessTimeStamp(*getCleanDirectory(directoryType, filePath));
}

FString UJSONParserFileFunctions::getFilenameOnDisk(EJSONDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetFilenameOnDisk(*getCleanDirectory(directoryType, filePath));
}

bool UJSONParserFileFunctions::createDirectory(EJSONDirectoryType directoryType, FString path) {
	return FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*getCleanDirectory(directoryType, path));
}


void UJSONParserFileFunctions::LoadFile(EJSONDirectoryType directoryType, FString filePath, FString& fileName, TArray<uint8>& bytes, int32& errorCode, FString& errorMessage) {

	bytes.Empty();

	if (filePath.IsEmpty()) {
		errorMessage = "FilePath not set.";
		errorCode = 1;
		return;
	}

	FString dir;
	if (directoryType == EJSONDirectoryType::E_ad) {
		dir = FPaths::ConvertRelativePathToFull(filePath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		dir = FPaths::ConvertRelativePathToFull(ProjectDir + filePath);
	}


	if (!FPaths::FileExists(dir)) {
		errorMessage = "File not found:" + dir;
		errorCode = 2;
		return;
	}


	if (!FFileHelper::LoadFileToArray(bytes, *dir)) {
		errorMessage = "Can't read File:" + dir;
		errorCode = 3;
		return;
	}

	fileName = FPaths::GetCleanFilename(filePath);

	errorCode = 0;
}

//void UJSONParserFileFunctions::LoadFileAsync(EJSONDirectoryType directoryType, FString filePath, FString eventID) {
//	if (loadFileThread == nullptr) {
//		loadFileThread = new FLoadFileThread(this, directoryType, filePath, eventID);
//	}
//	else {
//		loadFileThread->restartThread(this, directoryType, filePath, eventID);
//	}
//
//}

bool UJSONParserFileFunctions::LoadAllFilesFromDirectory(EJSONDirectoryType directoryType, FString fileType, FString dirPath,
	bool recursiveSearch, int32& count, TArray<FString>& files, TArray<FString>& filePaths) {

	FString dir;
	FString absoluteDir;
	if (directoryType == EJSONDirectoryType::E_ad) {
		if (dirPath.Len() < 1) {
			return false;
		}
		dir = FPaths::ConvertRelativePathToFull(dirPath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		dir = FPaths::ConvertRelativePathToFull(ProjectDir + dirPath);
	}

	absoluteDir = dir;
	if (absoluteDir.EndsWith("\\") == false && absoluteDir.EndsWith("/") == false)
		absoluteDir += "/";

	FPaths::NormalizeDirectoryName(dirPath);


	if (!FPaths::DirectoryExists(dir)) {
		return false;
	}
	IFileManager& FileManager = IFileManager::Get();

	dir += "/*."+fileType;
	
	if (recursiveSearch) {
		TArray<FString> filesFound;
		FileManager.FindFiles(filesFound, *dir, true, false);

		if (dirPath.Len() > 0 && dirPath.EndsWith("/") == false)
			dirPath += "/";
		//files
		for (int32 i = 0; i < filesFound.Num(); i++) {
			filePaths.Add(dirPath + filesFound[i]);
		}
		files.Append(filesFound);

		//dirs
		TArray<FString> dirsFound;
		FileManager.FindFiles(dirsFound, *(absoluteDir + "*"), false, true);
		for (int32 i = 0; i < dirsFound.Num(); i++) {
			LoadAllFilesFromDirectory(directoryType, fileType, (dirPath + dirsFound[i]), true, count, files, filePaths);
		}

	}
	else {
		FileManager.FindFiles(files, *dir, true, false);

		if (dirPath.Len() > 0)
			dirPath += "/";
		for (int32 i = 0; i < files.Num(); i++) {
			filePaths.Add(dirPath + files[i]);
		}
	}

	count = files.Num();
	return true;
}

bool UJSONParserFileFunctions::LoadAllDirectorysFromDirectory(EJSONDirectoryType directoryType, FString endsWith, FString dirPath,
	/*bool recursiveSearch,*/ int32& count, TArray<FString>& files, TArray<FString>& filePaths) {

	FString dir;
	FString absoluteDir;
	if (directoryType == EJSONDirectoryType::E_ad) {
		if (dirPath.Len() < 1) {
			return false;
		}
		dir = FPaths::ConvertRelativePathToFull(dirPath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		dir = FPaths::ConvertRelativePathToFull(ProjectDir + dirPath);
	}

	absoluteDir = dir;
	if (absoluteDir.EndsWith("\\") == false && absoluteDir.EndsWith("/") == false)
		absoluteDir += "/";

	FPaths::NormalizeDirectoryName(dirPath);


	if (!FPaths::DirectoryExists(dir)) {
		return false;
	}
	IFileManager& FileManager = IFileManager::Get();

	dir += "/*";

	//if (recursiveSearch) {
	//	TArray<FString> filesFound;
	//	FileManager.FindFiles(filesFound, *dir, false, true);

	//	if (dirPath.Len() > 0 && dirPath.EndsWith("/") == false)
	//		dirPath += "/";
	//	//files
	//	for (int32 i = 0; i < filesFound.Num(); i++) {
	//		filePaths.Add(dirPath + filesFound[i]);
	//	}
	//	files.Append(filesFound);

	//	//sub dirs
	//	TArray<FString> dirsFound;
	//	FileManager.FindFiles(dirsFound, *(absoluteDir + "*"), false, true);
	//	for (int32 i = 0; i < dirsFound.Num(); i++) {
	//		LoadAllFilesFromDirectory(directoryType, fileType, (dirPath + dirsFound[i]), true, count, files, filePaths);
	//	}

	//}
	//else {
		FileManager.FindFiles(files, *dir, false, true);

		if (dirPath.Len() > 0)
			dirPath += "/";
		for (int32 i = 0; i < files.Num(); i++) {
			if (files[i].EndsWith(endsWith)) {
				filePaths.Add(dirPath + files[i]);
			}
		}
	//}

	count = filePaths.Num();
	return true;
}

//void UJSONParserFileFunctions::LoadAllFilesFromDirectoryAsync(EJSONDirectoryType directoryType, FString fileType, FString dirPath, bool recursiveSearch, FString eventID) {
//	if (loadAllFilesFromDirectoryThread == nullptr) {
//		loadAllFilesFromDirectoryThread = new FLoadAllFilesFromDirectoryThread(this, directoryType, recursiveSearch, fileType, dirPath, eventID);
//	}
//	else {
//		loadAllFilesFromDirectoryThread->restartThread(this, directoryType, recursiveSearch, fileType, dirPath, eventID);
//	}
//}

void UJSONParserFileFunctions::SaveFile(EJSONDirectoryType directoryType, FString filePath, bool createDirectories, bool overwriteFile, TArray<uint8> bytes, FString& fileName, FString& fullPath, int32& errorCode, FString& errorMessage) {
	if (filePath.IsEmpty()) {
		errorMessage = "FilePath not set.";
		errorCode = 1;
		return;
	}

	fileName = FPaths::GetCleanFilename(filePath);
	if (fileName.Len() < 1 || fileName.Contains(".") == false) {
		errorMessage = "Missing Filename in this Filepath:" + filePath;
		errorCode = 6;
		return;
	}

	FString dir;
	if (directoryType == EJSONDirectoryType::E_ad) {
		dir = FPaths::ConvertRelativePathToFull(filePath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		dir = FPaths::ConvertRelativePathToFull(ProjectDir + filePath);
	}

	fullPath = dir;

	FString dirOnly = FPaths::GetPath(dir);
	if (!FPaths::DirectoryExists(dirOnly) && dirOnly.Len() > 2) {
		if (!createDirectories) {
			errorMessage = "Dir not found:" + dirOnly;
			errorCode = 2;
			return;
		}
		if (!FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(*dirOnly)) {
			errorMessage = "Can't create dir tree:" + dirOnly;
			errorCode = 3;
			return;
		}
	}

	if (FPaths::FileExists(dir)) {
		if (!overwriteFile) {
			errorMessage = "File already exists:" + dir;
			errorCode = 4;
			return;
		}
		else {
			if (!FFileHelper::SaveArrayToFile(bytes, *dir)) {
				errorMessage = "Can't write File:" + dir;
				errorCode = 5;
				return;
			}
		}
	}
	else {
		if (!FFileHelper::SaveArrayToFile(bytes, *dir)) {
			errorMessage = "Can't write File:" + dir;
			errorCode = 5;
			return;
		}
	}


	errorCode = 0;
}


FString UJSONParserFileFunctions::Base64Encode(TArray<uint8> bytes) {
	return FBase64::Encode(bytes);
}

TArray<uint8> UJSONParserFileFunctions::Base64Decode(FString base64EncodedString) {
	TArray<uint8> fileData;
	FBase64::Decode(*base64EncodedString, fileData);
	return fileData;
}




bool UJSONParserFileFunctions::base64StringToFile(EJSONDirectoryType directoryType, FString base64String, FString filePath) {

	if (base64String.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("JSONParser: Base64String not set."));
		return false;
	}

	if (filePath.IsEmpty()) {
		UE_LOG(LogTemp, Error, TEXT("JSONParser: FilePath not set."));
		return false;
	}

	FString dir = getCleanDirectory(directoryType, filePath);

	FString fileName = FPaths::GetCleanFilename(dir);
	FString tmpDir = dir;
	tmpDir.RemoveFromEnd(fileName);
	if (!FPaths::DirectoryExists(tmpDir)) {
		UE_LOG(LogTemp, Error, TEXT("JSONParser: Directory not exists: %s"), *tmpDir);
		return false;
	}

	TArray<uint8> fileData;
	if (!FBase64::Decode(*base64String, fileData)) {
		UE_LOG(LogTemp, Error, TEXT("JSONParser: Base64 decode failed: %s"), *dir);
		return false;
	}

	if (!FFileHelper::SaveArrayToFile(fileData, *dir)) {
		UE_LOG(LogTemp, Error, TEXT("JSONParser: Can't save File: %s"), *dir);
		return false;
	}

	return FPaths::FileExists(dir);
}

FString UJSONParserFileFunctions::Int64ToFString(int64 Num){

	FString string = FString();
	const TCHAR* DigitToChar = TEXT("9876543210123456789");
	constexpr int64 ZeroDigitIndex = 9;
	bool bIsNumberNegative = Num < 0;
	const int64 TempBufferSize = 16; 
	TCHAR TempNum[TempBufferSize];
	int64 TempAt = TempBufferSize; // fill the temp string from the top down.

	// Convert to string assuming base ten.
	do
	{
		TempNum[--TempAt] = DigitToChar[ZeroDigitIndex + (Num % 10)];
		Num /= 10;
	} while (Num);

	if (bIsNumberNegative)
	{
		TempNum[--TempAt] = TEXT('-');
	}

	const TCHAR* CharPtr = TempNum + TempAt;
	const int64 NumChars = TempBufferSize - TempAt;
	string.Append(CharPtr, NumChars);
	return string;
}

