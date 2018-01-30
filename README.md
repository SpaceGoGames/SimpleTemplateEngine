# Simple Template

Text based template engine with logic. Templates are compiled for better reuse and 
faster runtime usage.

## Install

Clone the whole repo into your local game `Plugins` folder or your engines `Plugins` folder.

TODO: Add some more info to make it easy, plus some precompiled binaries ^^.

## Templates

Templates are simple text files with a special syntax to define your needs. While simple it is yet quite powerful.

### Syntax

> {$VariableName}

Create a variable token. The name of the variable itself will be the key the interpreter will use to find the right data.

> {% for Item in ListKey %}Item: {$item}{% endfor %}

Will iterate a list list value that is stored in a key called `ListKey`. `Item` represents the element that we iterate. As you can see we print out the value of item using a variable token. Again, both `ListKey` and `Item` are keys use to look up for data.

The list iterator provides the index if needed, just use the `{$loop.index}` variable to use if your templates.

> {% if Engine == "UE4" %}Engine: Unreal Engine 4{% endif %}

A simple branch statement, you can either use literals as shown in the example or other keys. If the key is of boolean value you can even just use it for the branch. `==`, `!=` or `~=` are valid condition modiefiers. `~=` is used for special non-casesentitive conditions.

### Compiling

TODO: Just the whole process I guess xD

### Interpeting

Interpreting a template is quite simple, you can ither use a compiled template asset `USimpleTemplate` and call it's `FString Interpret(TScriptInterface<ISimpleTemplateDataProvider> DataProvider)` method or directly from a string.

A `ISimpleTemplateDataProvider` is just the interface you use to provide the engine with the needed data in JSON format. `USimpleTemplateData` is a the default data provider that comes out-of-the-box.

TODO: Low level stuff

## Usage

Once you have your templates setup you can use them either in C++ or in Blueprint using the provided function library.

TODO: Finish integrations and add documention

## Examples

TODO: Add some examples and screen shots

License
----

The MIT License (MIT)

Copyright (c) 2017 Playspace S.L.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Legal info
----

Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.

Unreal® Engine, Copyright 1998 – 2017, Epic Games, Inc. All rights reserved.
