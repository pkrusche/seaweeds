/***************************************************************************
 *   Copyright (C) 2009   Peter Krusche, The University of Warwick         *
 *   peter@dcs.warwick.ac.uk                                               *
 ***************************************************************************/
#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include <string>
#include <tbb/mutex.h>

#include "bspcpp/tools/utilities.h"
#include "bspcpp/ParameterFile.h"

namespace utilities {

template<class _checkpointer>
class GenericCheckpointable {
public:
	friend class Checkpointer;

	GenericCheckpointable() : cpt(NULL), myid("") {}

	/**
	 * @brief set the checkpointer used by this computation
	 * the checkpointer id can be used to distinguish multiple identical computations for checkpointing
	 */
	void set_checkpointer(_checkpointer * p, const char * _myid = "") {
		myid = _myid;
		cpt = p;
		new_checkpointer(p);
		if(cpt != NULL) {
			cpt->resume(*this, myid.c_str());
		}
	}

	/**
	 * @brief this function must be implemented to start running the computation from a checkpoint.
	 */
	virtual void run() = 0;

	/**
	 * @brief this function is called by checkpoint and allows the checkpointable class to save its state
	 */
	virtual void checkpoint_loadstate(ParameterFile & p) = 0;

	/**
 	 * @brief this function is called by checkpoint and allows the checkpointable class to save its state
	 */
	virtual void checkpoint_savestate(ParameterFile & p) = 0;

	/**
	 * @brief this function is called whenever the checkpointer is updated
	 */
	virtual void new_checkpointer(_checkpointer * ) {}

	/**
	 * @brief this function should be called by the checkpointable class whenever a safe state
	 * is reached.
	 */
	void checkpoint() {
		if(cpt != NULL) {
			cpt->checkpoint(*this, myid.c_str());
		}
	}

private:
	std::string myid;
	_checkpointer * cpt;
};

class Checkpointer {
public:
	Checkpointer(double _chkInterval = 5, bool _incremental = false) : 
	  filename(""), lastCheckpoint(time()), chkInterval(_chkInterval), incremental(_incremental) {}

	  /**
	  * @brief this function is called when a checkpointable object is resumed from a stored state
	  */
	  void initialize(const char* name, ParameterFile::storage_type storage = ParameterFile::ASCII) {
		  tbb::mutex::scoped_lock lock(my_mutex);

		  checkpointFile.clear();
		  if(!fexists(name)) {
			  // if file does not exist, initialize
			  checkpointFile.write(name, storage);
		  } else {
			  checkpointFile.read(name, storage);
		  }
		  filename = name;
		  storage_type = storage;
	  }

	  /**
	  * @brief checkpoint updates are written incrementally. flush reduces the checkpoint file to the final state
	  */
	  void flush() {
		  tbb::mutex::scoped_lock lock(my_mutex);

		  checkpointFile.write(filename.c_str(), storage_type);
		  lastCheckpoint = utilities::time();
		  std::cerr << "Checkpoint t = " << lastCheckpoint << std::endl;
	  }

	  /**
	  * @brief this is called by the checkpointable objects
	  */
	  void checkpoint(GenericCheckpointable<Checkpointer> & c, const char * id) {
		  tbb::mutex::scoped_lock lock(my_mutex);

		  checkpointFile.setprefix(id);
		  c.checkpoint_savestate(checkpointFile);
		  checkpointFile.setprefix();

		  if(filename != "" && lastCheckpoint + chkInterval < utilities::time()) {
			  if(incremental) {
				  checkpointFile.writeupdate(filename.c_str(), storage_type);
			  } else {
				  checkpointFile.write(filename.c_str(), storage_type);
			  }
			  lastCheckpoint = utilities::time();
			  std::cerr << "Checkpoint t = " << lastCheckpoint << std::endl;
		  }
	  }

	  /**
	  * @brief this is called by the checkpointable objects
	  */
	  void resume(GenericCheckpointable<Checkpointer> & c, const char * id) {
		  tbb::mutex::scoped_lock lock(my_mutex);
		  checkpointFile.setprefix(id);
		  c.checkpoint_loadstate(checkpointFile);
		  checkpointFile.setprefix();
	  }

private:
	double lastCheckpoint; ///< time last checkpoint was taken
	double chkInterval; ///< time interval for checkpoints
	bool incremental; ///< true if checkpoint file is incremental
	ParameterFile checkpointFile; ///< the checkpointing file
	std::string filename; ///< name of the checkpointing file
	ParameterFile::storage_type storage_type; ///< storage type for the parameter file

	tbb::mutex my_mutex;
};

typedef GenericCheckpointable<Checkpointer> Checkpointable;

};

#endif // __CHECKPOINT_H__

