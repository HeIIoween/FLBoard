# FLBoard #

A asynchronous API data communication hub plugin for FLHook and FLServer.

## How to compile ##

1. Get a Visual Studio 2013 (Express version also work)
1. Checkout the FLHook source code from The-Starport SVN server: http://svn.the-starport.net/flhookplugin/trunk/
3. Create a new folder under the **plugins** folder of your FLHook & source code directory *(Where you should also see another folder named **flhookplugin_sdk**)* and naming the folder **board**.
4. Checkout or download code from this repository on the **board** folder you just created.
5. Open up your Visual Studio 2013, and load FLHook solution by open **(PathToYourFLHookChecnout)\project-vc12\FLHook.vcxproj**.
6. Add this project into the FLHook solution by click **File** -> **Add** -> **Existing project**.
7. Once you done that, Visual Studio 2013 will automatically download the dependency libs of this plugin. Please wait until its completed.
8. Set project dependency by click **Project** -> **Project dependencies**. At the Project Dependencies window, select **Board Plugin** from the dropdown menu, and check **FLHook** on the Depends on checklist.
9. Click Build -> Build solution.

## JSON protocol ##

This plugin use JSON through HTTP POST requests to communicate with your API services.

To make the Board work for your API, you have to implememt following protocal in your API for communication. 

All communication based on this protocol which made with 3 basic type of instructs. 

### Login ###

#### FLServer side ####

When your FLServer has been boot up, Board will automatically send login signal to your FLServer as follow:

	{
		"Task": "Login",
		"Cecret": 0,
		"Account": "",
		"Password": "",
		"Queue": 0
	}

##### Task (string) #####

A mark for API server to recognize what type of request is it. In **Login** request, API server will needs to handle Login task.

##### Cecret (integer) #####

A random number use to identify the server feedback with a simple arithmetic.

##### Account (string) #####

The account that registered on current API server.

##### Password (string) #####

The password of that account.

##### Queue (integer) #####

How many queue item this server can send at one sync period.

The actual vaule of this number will be min(FLServerMaxQueue, APIServerMaxQueue).

You can config this number under [API] section of config files. When it be set to 0, FLServer will not send any queue to API server, neither receive. 

#### API server side ####

Once API server received a new login request, it will have to verify the Account name and password. 

If verification has passed, it will need to generate a token for later communication.

API server will return following JSON feedback to inform FLServer if login is succeed or failed.

	{
		"Ack": "Login",
		"Cecret": "0",
		"Token": "",
		"Name": "",
		"Error": "",
		"Delay": "0",
		"Queue": "0"
	}

##### HTTP Status #####

When FLServer's logon is succeed, API server has to send the JSON response under HTTP 200 status.

Any other status will be recognise as login failure.

##### Ack (string) #####

Just a simple feedback of the "Task" value in FLServer's request to make sure FLServer still waiting on this task.

##### Cecret (integer in string) #####

A random number use to identify the server feedback with a simple arithmetic.

To get the value of Cecret, use the **"Cecret"** value in FLServer's request, times it with a pre-negotiated value.

The pre-negotiated value can be generate when FLServer admin registering the FLServer on this API service. Once the value generated, FLServer admin needs to set this number to the **Secret =** setting under **[API]** section of config file.

However, if FLServer did't implement the **Secret =** setting, it will take any of Cecret value as true.

***Notice this number has to be send as string for FLServer to read, any non-string number will be recognize as 0***

##### Token (string) #####

A random string use to later on communication. FLServer will take this value and use it for future sync and logoff request. 

##### Name (string) #####

Friendly name of the API service mainly for player's recognition.

How to use this value will up to sub plugin implementing.

##### Error (string) #####

If there is a error you want to show explicitly on the FLHook console, you can set this value to explain for the reason of login failure.

When this value is not empty, FLServer will not consider login is succeed.

##### Delay (integer in string) #####

A limit delay that prevent FLServer from sending sync too often.

Set the Delay in second so FLServer will not send any other sync request within limiting time.

However, this not the time of FLServer heartbeat. FLServer not guarantee it will send a sync request for every delay period.

In Board's restrain, FLServer only syncs data when there has sync data to send. And data will be receive during same period. In other words, no data sent, no data receive. Sub plugin creator may implement their own heartbeat protocol with Update or similar Hooks to continuously send their own heartbeat sync package to get API feedback.

***Notice this number has to be send as string for FLServer to read, any non-string number will be recognize as 0***

##### Queue (integer in string) #####

The max number of queues this API can take. This for number negotiating the max amount of items in sync queue for receiving and also sending.

If API service returns 0, it will not receive any sync request.

***Notice this number has to be send as string for FLServer to read, any non-string number will be recognize as 0***

### Logoff ###

Log off request will happen when plugin unloading. It send for telling API server to release it's token and other resource.

Once a Logoff request has been proceesed, FLServer has to re-send the Login request to continue to communicate with API service.

#### FLServer side ####

FLServer will send following JSON request

	{
		"Task": "Logoff",
		"Cecret": 0,
		"Token": ""
	}

##### Task (string) #####

Work just like the one in the login request.

##### Cecret (integer) #####

Same as the one in login request.

##### Token (string) #####

The token used in current API communication that allocated previously by the API server at Login progress.

#### API server side ####

API server will send following JSON as feedback.

	{
		"Ack": "Logoff",
		"Cecret": "0"
	}

##### HTTP Status #####

API return 200 status as operation succeed. Any other status code will meanning failure.

##### Ack (string) #####

Acknowledgement of last task request.

##### Cecret (integer in string) #####

Same as the one in Login process.

### Sync ###

Sync request use to send data to API, and get data feedback in same time.

Sync request will be send when a plugin added data in sending queue by calling exported Event::Send() function.

#### FLServer side ####

FLServer will send the sync request when appropriate (Not in limit period + Has data to send).

Sync request will like follow:

	{
		"Task": "Sync",
		"Cecret": 0,
		"Token": ""
		"Datas": [
			{
				"Data" : {
					"DataItemName" : "DataItemValue"
					...
				},
				"Type" : "DataType"
			},
			{
				"Data" : {
					"DataItemName" : "DataItemValue"
				},
				"Type" : "DataType"
			},
			....
		]
	}

##### HTTP Status #####

HTTP Status must be 200 to represents succeed sync. Any other number will means failure.

##### Task (string) #####

Same as above requests

##### Cecret (integer) #####

Same as above requests

##### Datas (array) #####

The queue data that FLServer wants to send to API server. The max number of items will be limited by negotiated min number in **Login** request.

The data array for each item will be in this format:

	{
		"Data" : {
			"DataItemName" : "DataItemValue"
			...
		},
		"Type" : "DataType"
	},

###### Data (object) ######

The parameter set of this queue item, which can contains unlimited number of parameters.

The parameters will be in key => val pair for API server to process.

###### Type (string) ######

The type of this queue item.

This value gives a tag for API server for it to sort and process queue accorddingly.

#### API Server side ####

Once Sync request received, API will need send this JSON response as feedback.

	{
	    "Ack": "",
	    "Cecret": "0",
	    "Tasks": [
	        {
	            "Type": "TaskType",
	            "Data": {
	                "DataItemKey": "DataItemValue",
					.....
	            }
	        }
	    ]
	}

##### HTTP Status #####

HTTP Status must be 200 to represents succeed sync. Any other number will means failure.

##### Ack (string) #####

Same as aboving request

##### Cecret (integer in string) #####

Same as aboving request

##### Tasks (array) #####

The response queue for FLServer to receive.

The Task item will be in following format:

    {
        "Type": "Type",
        "Data": {
            "DataItemKey": "DataItemValue",
			.....
        }
    }

###### Type (string) ######

The type of this task queue item.

It uses as the key of FLServer responser.

Plugin creator needs to Listen request by bind a Callback function using Event::Listen(). Board plugin will automactially call the callback function one by one with received that type of task request item.

###### Data (object) ######

The task item data, which can contains unlimited number of parameters.

It will be use as the calling parameter for lister callback.

## Configuration file ##

### [General] section ###

#### Debug (yes / no) ####

The trigger to display / hide detailed running information.

#### Worker (integer) ####

Set how many working thread will be enabled.

The Board Plugin designed for working in one single thread. However, on a busy server or with a lively plugin, one thread could not handle all data well.

In that case, you may need to increase the worker number to adding more threads do to the job. 

Board will automatically call up the threads when needed, and sleep them when no work for them.

However, set the work number too high will cause lag of your FLServer.

### [API] section ###

#### URI ####

The URL of the API HTTP server

#### Account ####

Account name on that API server

#### Password ####

Password for that account

#### Secret ####

Pre-negotiated number for safe communication

#### Queue ####

How many queue we can take from this API server

#### Operation ####

What the data with this type we can send to API server. The value will be equals to

	....
	{
		"Data" : {
			"DataItemName" : "DataItemValue"
			...
		},
		"Type" : "**DataType**" // Operation type
	},
	....

in the sync request send by FLServer.

**Notice: If there is no Operation has set for this API, Board will take all as accepted.**

#### Response ####

What data types received from API server we will handle. The value will be equals to

	{
	    "Ack": "",
	    "Cecret": "0",
	    "Tasks": [
	        {
	            "Type": "**TaskType**", // Response Type
	            "Data": {
	                "DataItemKey": "DataItemValue",
					.....

in the sync request send by API server.

**Notice: If there is no Response has set for this API, Board will take all as accepted.**

### Example of configuration file ###

	[General]
	Debug = false
	Worker = 1
	
	[API]
	URI = http://the.address.to/your/api
	Account = TheAccountToLoginToTheAPI
	Password = ThePasswordForTheAccount
	Secret = 000 ; A number that only known by the API server and you
	Operation = PlayerLoggedIn
	Operation = PlayerLoggedOut
	Operation = PlayerTradelaneAccessed
	Operation = PlayerMessageOutbindReceived
	Response = Message
	
	[API] ; You can add another one
	URI = http://the.address.to/your/api
	Account = TheAccountToLoginToTheAPI
	Password = ThePasswordForTheAccount
	Secret = 000 ; A number that only known by the API server and you
	;Operation = PlayerLoggedIn
	;Operation = PlayerLoggedOut
	;Operation = PlayerTradelane.Accessed
	;Operation = PlayerMessageOutbindReceived
	;Response = Message

Notice you can import [API] sections from your plugin configuration directly to Board Plugin by calling

	Event::Import(scPluginCfgFile, true);
