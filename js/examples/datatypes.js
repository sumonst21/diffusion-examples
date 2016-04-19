/*******************************************************************************
 * Copyright (C) 2016, 2015 Push Technology Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/

var diffusion = require('diffusion');

// Connect to the server. Change these options to suit your own environment.
// Node.js does not accept self-signed certificates by default. If you have
// one of these, set the environment variable NODE_TLS_REJECT_UNAUTHORIZED=0
// before running this example.
diffusion.connect({
    host   : 'diffusion.example.com',
    port   : 443,
    secure : true,
    principal : 'control',
    credentials : 'password'
}).then(function(session) {

    // 1. Data Types are exposed from the top level Diffusion namespace. It is often easier
    // to assign these directly to a local variable.
    var jsonDataType = diffusion.datatypes.json();

    // 2. Data Types are currently provided for JSON and Binary topic types.
    session.topics.add('topic/json', diffusion.topics.TopicType.JSON);

    // 3. Values can be created directly from the data type.
    var jsonValue = jsonDataType.from({
        "foo" : "bar"
    });

    // Topics are updated using the standard update mechanisms
    session.topics.update('topic/json', jsonValue);

    // Subscriptions are performed normally
    session.subscribe('topic/json');

    // 4. Streams can be specialised to provide values from a specific datatype.
    session.stream('topic/json').asType(jsonDataType).on('value', function(topic, specification, newValue, oldValue) {
        // When a JSON or Binary topic is updated, any value handlers on a subscription will be called with both the
        // new value, and the old value.
   
        // The oldValue parameter will be undefined if this is the first value received for a topic.

        // For JSON topics, value#get returns a JavaScript object
        // For Binary topics, value#get returns a Buffer instance
        console.log("Update for " + topic, newValue.get());
    });

    // 5. Raw values of an appropriate type can also be used for JSON and Binary topics. 
    // For example, plain JSON objects can be used to update JSON topics.
    session.topics.update('topic/json', {
         "foo" : "baz",
         "numbers" : [1, 2, 3]
    });
});
