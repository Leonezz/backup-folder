#pragma once
//sync status flag
enum SyncStatus {
	//task is syncing
	Syncing,
	//task is checking differents
	Checking,
	//task succeed
	Finished,
	//task failed
	Failed
};