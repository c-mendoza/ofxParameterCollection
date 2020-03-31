//
// Created by Cristobal Mendoza on 3/16/20.
//

#ifndef OFX_PARAMETER_COLLECTION_H
#define OFX_PARAMETER_COLLECTION_H

#include <ofParameter.h>

/**
 * @brief ofxParameterCollection allows you to have an indefinite number of ofParameters of the same type while
 * preserving their serialization and notification abilities. The class is useful in situations where you would
 * like to work with ofParameters but you don't know ahead of time how many parametrized items you will have. In
 * essence, the class is like an std::vector for ofParameters, and its use cases are similar to that of std::vector.
 *
 * @tparam ParameterType: The data type that the ofParameters will wrap. Note that the collection stores items of a
 * single type, so it is a lot more like an std::vector than an ofParameterGroup.
 *
 * The class handles serialization by creating an ofParameterGroup that will hold the ofParameters, and by assigning
 * sequential names to each item in the collection. Adding or removing items can be listened via the
 * collectionChangedEvent, while item value changes are notified via the collectionItemChangedEvent.
 */
template<typename ParameterType>
class ofxParameterCollection
{
protected:
	std::string itemPrefix;
	ofParameterGroup parameterGroup;
	bool isSetup = false;
	bool hasLimits = false;
	std::vector<std::shared_ptr<ofParameter<ParameterType>>> parameters;
	ofEventListeners valueListeners;
	ParameterType min;
	ParameterType max;
public:

	/**
	 * @brief Subscribe to this event to be notified when items are added or removed from the collection.
	 * The event handler signature should be (ofxParameterCollection<yourCollectionType>& pCollection)
	 *
	 */
	ofEvent<ofxParameterCollection<ParameterType>> collectionChangedEvent;

	/**
	 * @brief Subscribe to this event to be notified when the value of an ofParameter in the collection changes.
	 *
	 * The event handler signature should be (ofParameter<yourCollectionType>& param)
	 * ex:
	 *
	 * ofxParameterCollection<int> myInts;
	 * myInts.setup("integer", "MyIntegers");
	 *
	 * auto eventListener = myInts.collectionItemChangedEvent.newListener([](ofParameter<int>& theParam) {
	 * 		//do something with the param
	 * 	});
	 *
	 * 	// Make sure to store that eventListener somewhere!
	 */

	ofEvent<ofParameter<ParameterType>> collectionItemChangedEvent;

	/**
	 * @brief Readies the collection for use. Call this method prior to any other in the class.
	 * @param itemPrefix The std::string that will be prefixed to all of the entries in the collection's
	 * ofParameterGroup. Ex: if your prefix is "My Param ", the entries in the parameter group will be (after escaping)
	 * "My_Param_0", "My_Param_1", etc.
	 * @param groupName The name that will be assigned to the collection's ofParameterGroup.
	 * @param parentGroup The group where the ofxParameterCollection's parameterGroup will be placed in.
	 *
	 */
	void setup(std::string itemPrefix, std::string groupName, ofParameterGroup& parentGroup)
	{
		this->itemPrefix = itemPrefix;
		parameterGroup.setName(groupName);
		parentGroup.add(parameterGroup);
		isSetup = true;
	}

	/**
	 * @brief Readies the collection for use. Call this method prior to any other in the class. Use this version
	 * of the method to set the minimum and maximum for the ofParameters' values.
	 * @param itemPrefix The std::string that will be prefixed to all of the entries in the collection's
	 * ofParameterGroup. Ex: if your prefix is "My Param ", the entries in the parameter group will be (after escaping)
	 * "My_Param_0", "My_Param_1", etc.
	 * @param groupName The name that will be assigned to the collection's ofParameterGroup.
	 * @param parentGroup The group where the ofxParameterCollection's parameterGroup will be placed in.
	 * @param min The minimum value for the ofParameter
	 *
	 */
	void setup(std::string itemPrefix, std::string groupName, ofParameterGroup& parentGroup, ParameterType min,
			   ParameterType max)
	{
		setup(itemPrefix, groupName, parentGroup);
		setLimits(min, max);
	}

	/**
	 * @brief Sets the minimum and maximum for the ofParameters' values.
	 */
	void setLimits(ParameterType min, ParameterType max)
	{
		this->min = min;
		this->max = max;
		hasLimits = true;
	}

	/**
	 * @brief Creates an ofParameter with the supplied value and adds it to the collection.
	 * @param value The value that the ofParameter will be assigned.
	 * @param notify If true, notifies the collectionChangedEvent listeners. This is the default behavior.
	 */
	void addItem(ParameterType value, bool notify = true)
	{
		assert(isSetup);
		ofParameter<ParameterType> param;
		param.set(itemPrefix + ofToString(parameterGroup.size()),
				  value);

		if (hasLimits)
		{
			param.setMin(min);
			param.setMax(max);
		}

		auto paramPtr = std::make_shared<ofParameter<ParameterType>>(param);

		valueListeners.push(paramPtr->newListener([&, paramPtr](ParameterType& value)
												  {
													  collectionItemChangedEvent.notify(*paramPtr);
												  }));

		parameters.push_back(paramPtr);
		parameterGroup.add(*paramPtr);
		assert(parameters.size() == parameterGroup.size());
		if (notify) collectionChangedEvent.notify(*this);
	}

	// TODO
//	void addItemAt(ofAbstractParameter& param, int i, bool notify = true)
//	{}

/**
 * @brief Gets a reference to the ofParameter at the given index.
 */
	std::shared_ptr<ofParameter<ParameterType>> getAt(int index)
	{
		return parameters.at(index);
	}

	
	bool removeAt(int i, bool notify = true)
	{
		if (i >= parameters.size())
		{
			ofLogNotice("ofxParameterCollection") << "removeAt: Index out of bounds. Index: " << i;
			return false;
		}

		return removeItem(getAt(i), notify);
	}

	bool removeItem(std::shared_ptr<ofParameter<ParameterType>> param, bool notify = true)
	{
		auto iter = std::find(parameters.begin(), parameters.end(), param);
		if (iter != parameters.end())
		{
			parameters.erase(iter);
			// Sadly we can't delete single params from the group and rename them, otherwise
			// ofParameterGroup loses track of it. So we use setCollection to clear the group
			// and re-add all our items. On the upside, we leave no dangling event listeners.
			setCollection(parameters, false);
			if (notify) collectionChangedEvent.notify(*this);
			assert(parameterGroup.size() == parameters.size());
			return true;
		}

		return false;
	}
	
	void setCollection(std::vector<std::shared_ptr<ofParameter<ParameterType>>> newCollection, bool notify = true)
	{
		this->clear(false);
		for (auto& paramPtr : newCollection)
		{
			addItem(*paramPtr.get(), false);
		}
		if (notify) this->notify();
	}

	/**
	 * @brief Clears and rebuilds the ParameterCollection using the supplied values.
	 * All parameters previously in the collection are removed, so make sure that you are not
	 * relying on listening for value changes in individual parameters (i.e. if you add your own listener
	 * to the parameter it will be destroyed).
	 * The listeners to the collectionChangedEvent and collectionItemChangedEvent are not affected.
	 * @param newCollection
	 */
	void setCollection(std::vector<std::shared_ptr<ParameterType>> newCollection, bool notify = true)
	{
		this->clear(false);
		for (auto& paramPtr : newCollection)
		{
			addItem(*paramPtr, false);
		}
		if (notify) this->notify();
	}

	/**
	 * @brief Clears and rebuilds the ParameterCollection using the supplied vector of values.
	 * All parameters previously in the collection are removed, so make sure that you are not
	 * relying on listening for value changes in individual parameters.
	 * The listeners to the collectionChangedEvent are not affected.
	 * @param newCollection
	 */
	void setCollection(std::vector<ParameterType> newCollection, bool notify = true)
	{
		this->clear(false);
		for (auto& value : newCollection)
		{
			addItem(value, false);
		}
		if (notify) this->notify();
	}

	/**
	 * @brief Sets the values of the ofParameters in the collection. The @param newValues size() must be
	 * equal to the number of ofParameters currently in the collection.
	 * @param newValues
	 */
	void setValues(std::vector<std::shared_ptr<ParameterType>> newValues, bool notify = true)
	{
		assert(newValues.size() == parameters.size());
		for (int i = 0; i < parameters.size(); i++)
		{
			parameters[i].get() = *(newValues[i]);
		}
		if (notify) this->notify();
	}

	/**
	 * @brief Sets the values of the ofParameters in the collection. The @param newValues size() must be
	 * equal to the number of ofParameters currently in the collection.
	 * @param newValues
	 */
	void setValues(std::vector<ParameterType> newValues, bool notify = true)
	{
		assert(newValues.size() == parameters.size());
		for (int i = 0; i < parameters.size(); i++)
		{
			parameters[i].get() = newValues[i];
		}
		if (notify) this->notify();
	}

	/**
	 * @brief Begin iterator for the ofParameters in the collection.
	 * @return
	 */
	typename std::vector<std::shared_ptr<ofParameter<ParameterType>>>::const_iterator begin()
	{
		assert(parameters.size() == parameterGroup.size());
		return parameters.begin();
	}

	/**
	 * @brief End iterator for the ofParameters in the collection.
	 * @return
	 */
	typename std::vector<std::shared_ptr<ofParameter<ParameterType>>>::const_iterator end()
	{
		assert(parameters.size() == parameterGroup.size());
		return parameters.end();
	}

	/**
	 * @brief Returns the number of ofParameters in the collection.
	 * @return
	 */
	size_t size()
	{
		assert(parameters.size() == parameterGroup.size());
		return parameters.size();
	}

	/**
	 * @brief Returns a reference to the last ofParameter in the collection.
	 */
	std::shared_ptr<ofParameter<ParameterType>>& back()
	{
		return parameters.back();
	}

	/**
	 * @brief Removes all ofParameters from the group.
	 * @param notify If true, notifies the collectionChangedEvent listeners. This is the default behavior.
	 */
	void clear(bool notify = true)
	{
		// I'm not calling parameterGroup.clear() because I was concerned that it would clear
		// its value pointer and muck up the Parameter tree elsewhere. Testing would clear that up, but
		// this works for the moment
		for (int i = parameterGroup.size() - 1; i >= 0; i--)
		{
			parameterGroup.remove(i);
		}
		parameters.clear();
		valueListeners.unsubscribeAll();
		if (notify) collectionChangedEvent.notify(*this);
	}

	/**
	 * @brief Gets you the ofParameterGroup that includes the ofParameters in the collection.
	 * @return The ofParameterGroup.
	 *
	 * The ofParameterGroup is populated and managed by ofxParameterCollection. Do not add or remove items
	 * from the ofParameterGroup yourself, this might break things. If you want to add or remote items to the collection,
	 * use addItem or removeItem
	 */
	ofParameterGroup& getGroup()
	{
		assert(isSetup);
		return parameterGroup;
	}

	/**
	 * @brief Call this method prior to deserializing the ofxParameterCollection. IF YOU DON'T CALL THIS METHOD
	 * DESERIALIZATION WILL NOT WORK! See the example for usage, but the long and short of it is that you should
	 * call this method prior to calling ofDeserialize from your app.
	 * @param xml The XML root from which to start searching for the parameter group.
	 * @param clear True by default. Clears the parameter group and the underlying data storage prior to
	 * pre-deserialization. Not sure why you would not want to clear the backing storage, but it's an option.
	 *
	 * Why do we need to pre-deserialize?
	 * OF deserializes by loading a serialized file and then trying to match tag names to ofParameter names. This
	 * works great when you know all of your parameters ahead of time, but when we are working with collections,
	 * by definition, we don't know how many elements we'll end up having, and thus we can't know the names of
	 * the parameters we are trying to match. This method looks into the XML file and finds how
	 * many XML elements with the specified collection prefix are in an element with the name of our parameterGroup.
	 * It will then insert into the collection that number of ofParameters, naming them sequentially with our prefix,
	 * adding them to the parameterGroup as well. Once we have properly named parameters in the group that match the
	 * XML file, we can proceed to deserialize. Note that when you predeserialize the created parameters have null
	 * values. You should call ofDeserialize right after preDeserialize to assign values to the created parameters.
	 */
	void preDeserialize(ofXml& xml, bool clear = true)
	{
		assert(isSetup);

		if (clear) this->clear(false); // Don't notify, since we are going to deserialize soon

//		auto root = xml.findFirst("//" + parameterGroup.getEscapedName());
		auto path = "//" + parameterGroup.getEscapedName();


		auto search = xml.findFirst(path);

		if (!search)
		{
			ofLogNotice(__PRETTY_FUNCTION__) << "Could not find " << path;
			return;
		}
		auto children = search.getChildren();
		for (auto child : children)
		{
			ofLogVerbose(__PRETTY_FUNCTION__) << child.getName();
			if (child.getValue().size() == 0)
			{
				ofLogError(__PRETTY_FUNCTION__) << "Ignoring empty child in group " << parameterGroup.getName();
				continue;
			}
			addEntry(false);
		}
	}

	/**
	 * @brief Returns a copy of the parameter storage vector. Note that modifying this vector does not change
	 * the internal state of the collection. If you want to iterate over the collection, consider using the
	 * begin() and end() iterators of the class, or use a range based for loop.
	 */
	std::vector<std::shared_ptr<ofParameter<ParameterType>>> getParameters() const
	{
		return parameters;
	}

	/**
	 * @brief Notifies the listeners of the collectionChangedEvent. You shouldn't have to call this yourself
	 * in most situations.
	 */
	void notify()
	{
		collectionChangedEvent.notify(*this);
	}

protected:
	/**
 	* @brief Creates an ofParameter with a null value and adds it the collection. This is mostly useful to get
 	* the ofParameterGroup ready for deserialization, so you shouldn't have to call this method.
 	* @param notify determines whether the collectionChangedEvent fires. The default is true.
 	*/
	void addEntry(bool notify = true)
	{
		ParameterType value;
		addItem(value, notify);
	}

	/**
	 * @brief Creates a number of ofParameters with a null value and adds them to the collection. This
	 * is mostly useful to get the ParameterGroup ready for deserialization, so you shouldn't have to call this method.
	 * @param count
	 * @param notify determines whether the collectionChangedEvent fires. The default is true.
	 */
	void addEntries(int count, bool notify = true)
	{
		for (int i = 0; i < count; i++)
		{
			addEntry(notify);
		}
	}
};


#endif //OFX_PARAMETER_COLLECTION_H
