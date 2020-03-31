# ofxParameterCollection
ofxParameterCollection is an [openFrameworks](http://openframeworks.cc) addon that manages an indefinite number of ofParameters of the same type while providing simple serialization, deserialization and notification. The class is useful in situations where you would like to work with ofParameters but you don't know ahead of time how many parameterized items you will have. In essence, the class is like an std::vector for ofParameters, and its use cases are similar to that of std::vector.

## Installation
It installs like any other OF addon, but since it is a header-only library consisting of a single file you can just copy it into your project.

## Concept
I love working with ofParameters, they serialize and deserialize easily and their notification system is great. However, one situation in which they are not so great is when you need a variable number of ofParameters that you want to be able to save. Because of the way they are deserialized, ofParameters need to be declared and named before ofDeserialize is called, and this makes it impossible to just have an empty vector of ofParameters that OF could populate based on the contents of the serialized file. ofxParameterCollection was created to solve this issue.

It does the following:
* It manages a collection of ofParameters of the same type based on values you provide. You can add and remove items from the collection, and the collection state is saved when it is serialized.
* Manages the deserialization of the collection. Serialization works without any additional work!
* Notifies event listeners when the collection changes, or when items in the collection change values.

## Usage
`ofxParameterCollection` works a bit like an `std::vector` (in fact, the `ofParameters` are stored in a vector), so it only handles items of the same type.

First declare (in the header most likely) the collection with the type to store:

```C++
ofxParameterCollection<float> myParams;

// Let's assume that we also have a parameter group where we
// are storing our other parameters:
ofParameterGroup mainParameterGroup;
```

Then you need to setup the collection:

```C++
myParams.setup("My Param ", "My Param Collection", mainParameterGroup);
```

The first function parameter is a string that will be the "prefix" used by `ofxParameterCollection` to name each `ofParameter`. Thus, in this case your `ofParameters` would be named `"My Param 0"`, `"My Param 1"`, etc. The second function parameter is a string to name the `ofParameterGroup` that `ofxParameterCollection` will create for you, where the collection's `ofParameters` will be placed. The third function parameter is the parent `ofParameterGroup` where the group created by `ofxParameterCollection` will be placed. In the simplest cases, the parent group is the one that you will pass along to `ofDeserialize` and `ofSerialize`.

A few things to note about naming:
* These names will be escaped by OF when serializing, so `"My Param 0"` will be `"My_Param_0"` in the XML file.
* The name for the `ofParameterGroup` must not conflict with anything else in your main parameter group.

### (De)serialization
To deserialize, you ***must*** call `preDeserialize` prior to calling `ofDeserialize`. This sets up the parameter groups so that the deserializer finds our collection:
```C++
myParams.preDeserialize(yourDeserializer);
ofDeserialize(yourDeserializer, mainParameterGroup);
```

You don't need to do anything extra for serialization, just call ofSerialize as you would have normally:
```C++
ofSerialize(yourDeserializer, mainParameterGroup);
```
### Managing Items
You create new ofParameters in the collection by calling `addItem(ParameterType value)`. You don't need to create the ofParameter yourself, the class handles that for you. Delete items from the collection by using `removeItem(ofParameter<ParameterType>)`. You can also iterate over the items by using the `begin()` and `end()` iterators, or you can also use a range-based for loop:
```C++
// Note that param is a shared_ptr!!
for (auto& param : myVectorParams) {
	ofDrawEllipse(*param, 100, 100);
}
```

### Events
The class provides two events that you can listen to:
* `collectionChangedEvent` notifies when items are added or removed from the collection.
* `collectionItemChangedEvent` notifies when the value of an ofParameter in the collection changes. See the example for more details.

Make sure to check out the example included in the repo.

## Version
0.2
