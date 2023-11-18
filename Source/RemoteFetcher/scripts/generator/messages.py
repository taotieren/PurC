# Copyright (C) 2010-2020 Apple Inc. All rights reserved.
# Copyright (C) 2019 Beijing FMSoft Technologies Co., Ltd.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import collections
import re
import sys

from generator import parser

WANTS_CONNECTION_ATTRIBUTE = 'WantsConnection'
WANTS_DISPATCH_MESSAGE_ATTRIBUTE = 'WantsDispatchMessage'
LEGACY_RECEIVER_ATTRIBUTE = 'LegacyReceiver'
NOT_REFCOUNTED_RECEIVER_ATTRIBUTE = 'NotRefCounted'
SYNCHRONOUS_ATTRIBUTE = 'Synchronous'
ASYNC_ATTRIBUTE = 'Async'

_license_header = """/*
 * Copyright (C) 2010-2020 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

"""


def messages_header_filename(receiver):
    return '%sMessages.h' % receiver.name


def surround_in_condition(string, condition):
    if not condition:
        return string
    return '#if %s\n%s#endif\n' % (condition, string)


def function_parameter_type(type, kind):
    # Don't use references for built-in types.
    builtin_types = frozenset([
        'bool',
        'float',
        'double',
        'uint8_t',
        'uint16_t',
        'uint32_t',
        'uint64_t',
        'int8_t',
        'int16_t',
        'int32_t',
        'int64_t',
    ])

    if type in builtin_types:
        return type

    if kind.startswith('enum:'):
        return type

    return 'const %s&' % type


def reply_parameter_type(type):
    return '%s&' % type


def move_type(type):
    return '%s&&' % type


def arguments_type(message):
    return 'std::tuple<%s>' % ', '.join(function_parameter_type(parameter.type, parameter.kind) for parameter in message.parameters)


def reply_type(message):
    return 'std::tuple<%s>' % (', '.join(reply_parameter_type(parameter.type) for parameter in message.reply_parameters))


def reply_arguments_type(message):
    return 'std::tuple<%s>' % (', '.join(parameter.type for parameter in message.reply_parameters))


def message_to_reply_forward_declaration(message):
    result = []

    if message.reply_parameters != None and (message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(ASYNC_ATTRIBUTE)):
        send_parameters = [(function_parameter_type(x.type, x.kind), x.name) for x in message.reply_parameters]
        completion_handler_parameters = '%s' % ', '.join([' '.join(x) for x in send_parameters])

        if message.has_attribute(ASYNC_ATTRIBUTE):
            result.append('using %sAsyncReply' % message.name)
        else:
            result.append('using %sDelayedReply' % message.name)
        result.append(' = CompletionHandler<void(%s)>;\n' % completion_handler_parameters)

    if not result:
        return None

    return surround_in_condition(''.join(result), message.condition)


def message_to_struct_declaration(receiver, message):
    result = []
    function_parameters = [(function_parameter_type(x.type, x.kind), x.name) for x in message.parameters]

    result.append('class %s {\n' % message.name)
    result.append('public:\n')
    result.append('    using Arguments = %s;\n' % arguments_type(message))
    result.append('\n')
    result.append('    static IPC::MessageName name() { return IPC::MessageName::%s_%s; }\n' % (receiver.name, message.name))
    result.append('    static const bool isSync = %s;\n' % ('false', 'true')[message.reply_parameters != None and not message.has_attribute(ASYNC_ATTRIBUTE)])
    result.append('\n')
    if message.reply_parameters != None:
        send_parameters = [(function_parameter_type(x.type, x.kind), x.name) for x in message.reply_parameters]
        completion_handler_parameters = '%s' % ', '.join([' '.join(x) for x in send_parameters])
        if message.has_attribute(ASYNC_ATTRIBUTE):
            move_parameters = ', '.join([move_type(x.type) for x in message.reply_parameters])
            result.append('    static void callReply(IPC::Decoder&, CompletionHandler<void(%s)>&&);\n' % move_parameters)
            result.append('    static void cancelReply(CompletionHandler<void(%s)>&&);\n' % move_parameters)
            result.append('    static IPC::MessageName asyncMessageReplyName() { return IPC::MessageName::%s_%sReply; }\n' % (receiver.name, message.name))
            result.append('    using AsyncReply = %sAsyncReply;\n' % message.name)
        elif message.has_attribute(SYNCHRONOUS_ATTRIBUTE):
            result.append('    using DelayedReply = %sDelayedReply;\n' % message.name)
        if message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(ASYNC_ATTRIBUTE):
            result.append('    static void send(std::unique_ptr<IPC::Encoder>&&, IPC::Connection&')
            if len(send_parameters):
                result.append(', %s' % completion_handler_parameters)
            result.append(');\n')
        result.append('    using Reply = %s;\n' % reply_type(message))
        result.append('    using ReplyArguments = %s;\n' % reply_arguments_type(message))

    if len(function_parameters):
        result.append('    %s%s(%s)' % (len(function_parameters) == 1 and 'explicit ' or '', message.name, ', '.join([' '.join(x) for x in function_parameters])))
        result.append('\n        : m_arguments(%s)\n' % ', '.join([x[1] for x in function_parameters]))
        result.append('    {\n')
        result.append('    }\n\n')
    result.append('    const Arguments& arguments() const\n')
    result.append('    {\n')
    result.append('        return m_arguments;\n')
    result.append('    }\n')
    result.append('\n')
    result.append('private:\n')
    result.append('    Arguments m_arguments;\n')
    result.append('};\n')
    return surround_in_condition(''.join(result), message.condition)


def forward_declaration(namespace, kind_and_type):
    kind, type = kind_and_type

    qualified_name = '%s::%s' % (namespace, type)
    if kind == 'struct':
        return 'struct %s' % type
    elif kind.startswith('enum:'):
        return 'enum class %s : %s' % (type, kind[5:])
    else:
        return 'class %s' % type


def forward_declarations_for_namespace(namespace, kind_and_types):
    result = []
    result.append('namespace %s {\n' % namespace)
    result += ['%s;\n' % forward_declaration(namespace, x) for x in sorted(kind_and_types)]
    result.append('}\n')
    return ''.join(result)


def types_that_cannot_be_forward_declared():
    return frozenset([
        'MachSendRight',
        'MediaTime',
        'String',
        'PurCFetcher::ColorSpace',
        'PurCFetcher::DictationContext',
        'PurCFetcher::DocumentIdentifier',
        'PurCFetcher::DocumentOrWorkerIdentifier',
        'PurCFetcher::FetchIdentifier',
        'PurCFetcher::FrameIdentifier',
        'PurCFetcher::LibWebRTCSocketIdentifier',
        'PurCFetcher::MediaSessionIdentifier',
        'PurCFetcher::PageIdentifier',
        'PurCFetcher::PluginLoadClientPolicy',
        'PurCFetcher::PointerID',
        'PurCFetcher::ProcessIdentifier',
        'PurCFetcher::RealtimeMediaSourceIdentifier',
        'PurCFetcher::RenderingMode',
        'PurCFetcher::ServiceWorkerIdentifier',
        'PurCFetcher::ServiceWorkerJobIdentifier',
        'PurCFetcher::ServiceWorkerOrClientData',
        'PurCFetcher::ServiceWorkerOrClientIdentifier',
        'PurCFetcher::ServiceWorkerRegistrationIdentifier',
        'PurCFetcher::SharedStringHash',
        'PurCFetcher::SleepDisablerIdentifier',
        'PurCFetcher::SWServerConnectionIdentifier',
        'PurCFetcher::ActivityStateChangeID',
        'PurCFetcher::AudioMediaStreamTrackRendererIdentifier',
        'PurCFetcher::ContentWorldIdentifier',
        'PurCFetcher::DisplayLinkObserverID',
        'PurCFetcher::GeolocationIdentifier',
        'PurCFetcher::ImageBufferBackendHandle',
        'PurCFetcher::ImageBufferFlushIdentifier',
        'PurCFetcher::ImageBufferIdentifier',
        'PurCFetcher::LayerHostingContextID',
        'PurCFetcher::LegacyCustomProtocolID',
        'PurCFetcher::LibWebRTCResolverIdentifier',
        'PurCFetcher::MDNSRegisterIdentifier',
        'PurCFetcher::MediaPlayerPrivateRemoteIdentifier',
        'PurCFetcher::MediaRecorderIdentifier',
        'PurCFetcher::PluginProcessType',
        'PurCFetcher::RemoteAudioDestinationIdentifier',
        'PurCFetcher::RemoteAudioSessionIdentifier',
        'PurCFetcher::RemoteCDMIdentifier',
        'PurCFetcher::RemoteCDMInstanceIdentifier',
        'PurCFetcher::RemoteCDMInstanceSessionIdentifier',
        'PurCFetcher::RemoteLegacyCDMIdentifier',
        'PurCFetcher::RemoteLegacyCDMSessionIdentifier',
        'PurCFetcher::RemoteMediaResourceIdentifier',
        'PurCFetcher::RenderingBackendIdentifier',
        'PurCFetcher::RTCDecoderIdentifier',
        'PurCFetcher::RTCEncoderIdentifier',
        'PurCFetcher::SampleBufferDisplayLayerIdentifier',
        'PurCFetcher::StorageAreaIdentifier',
        'PurCFetcher::StorageAreaImplIdentifier',
        'PurCFetcher::StorageNamespaceIdentifier',
        'PurCFetcher::TrackPrivateRemoteIdentifier',
        'PurCFetcher::TransactionID',
        'PurCFetcher::UserContentControllerIdentifier',
        'PurCFetcher::WebPageProxyIdentifier',
        'PurCFetcher::WebSocketIdentifier',
    ])


def conditions_for_header(header):
    conditions = {
        '"InputMethodState.h"': ["PLATFORM(GTK)", "PLATFORM(WPE)", "PLATFORM(HBD)"],
        '"LayerHostingContext.h"': ["PLATFORM(COCOA)", ],
        '"GestureTypes.h"': ["PLATFORM(IOS_FAMILY)"],
    }
    if not header in conditions:
        return None
    return conditions[header]


def forward_declarations_and_headers(receiver):
    types_by_namespace = collections.defaultdict(set)

    headers = set([
        '"ArgumentCoders.h"',
        '"Connection.h"',
        '"MessageNames.h"',
        '<wtf/Forward.h>',
        '<wtf/ThreadSafeRefCounted.h>',
        '"%sMessagesReplies.h"' % receiver.name,
    ])

    non_template_wtf_types = frozenset([
        'MachSendRight',
        'MediaType',
        'String',
    ])

    no_forward_declaration_types = types_that_cannot_be_forward_declared()
    for parameter in receiver.iterparameters():
        kind = parameter.kind
        type = parameter.type

        if type.find('<') != -1 or type in no_forward_declaration_types:
            # Don't forward declare class templates.
            headers.update(headers_for_type(type))
            continue

        split = type.split('::')

        # Handle WTF types even if the WTF:: prefix is not given
        if split[0] in non_template_wtf_types:
            split.insert(0, 'WTF')

        if len(split) == 2:
            namespace = split[0]
            inner_type = split[1]
            types_by_namespace[namespace].add((kind, inner_type))
        elif len(split) > 2:
            # We probably have a nested struct, which means we can't forward declare it.
            # Include its header instead.
            headers.update(headers_for_type(type))

    forward_declarations = '\n'.join([forward_declarations_for_namespace(namespace, types) for (namespace, types) in sorted(types_by_namespace.items())])

    header_includes = []
    for header in sorted(headers):
        conditions = conditions_for_header(header)
        if conditions and not None in conditions:
            header_include = '#if %s\n' % ' || '.join(sorted(set(conditions)))
            header_include += '#include %s\n' % header
            header_include += '#endif\n'
            header_includes.append(header_include)
        else:
            header_includes.append('#include %s\n' % header)

    return (forward_declarations, header_includes)


def forward_declarations_and_headers_for_replies(receiver):
    types_by_namespace = collections.defaultdict(set)

    headers = set([
        '<wtf/Forward.h>',
        '"MessageNames.h"',
    ])

    non_template_wtf_types = frozenset([
        'MachSendRight',
        'MediaTime',
        'String',
    ])

    no_forward_declaration_types = types_that_cannot_be_forward_declared()
    for message in receiver.messages:
        if message.reply_parameters == None or not (message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(ASYNC_ATTRIBUTE)):
            continue

        for parameter in message.reply_parameters:
            kind = parameter.kind
            type = parameter.type

            if type.find('<') != -1 or type in no_forward_declaration_types:
                # Don't forward declare class templates.
                headers.update(headers_for_type(type))
                continue

            split = type.split('::')

            # Handle WTF types even if the WTF:: prefix is not given
            if split[0] in non_template_wtf_types:
                split.insert(0, 'WTF')

            if len(split) == 2:
                namespace = split[0]
                inner_type = split[1]
                types_by_namespace[namespace].add((kind, inner_type))
            elif len(split) > 2:
                # We probably have a nested struct, which means we can't forward declare it.
                # Include its header instead.
                headers.update(headers_for_type(type))

    forward_declarations = '\n'.join([forward_declarations_for_namespace(namespace, types) for (namespace, types) in sorted(types_by_namespace.items())])

    header_includes = []
    for header in sorted(headers):
        conditions = conditions_for_header(header)
        if conditions and not None in conditions:
            header_include = '#if %s\n' % ' || '.join(sorted(set(conditions)))
            header_include += '#include %s\n' % header
            header_include += '#endif\n'
            header_includes.append(header_include)
        else:
            header_includes.append('#include %s\n' % header)

    return (forward_declarations, header_includes)


def generate_messages_reply_header(receiver):
    result = []

    result.append(_license_header)

    result.append('#pragma once\n')
    result.append('\n')

    if receiver.condition:
        result.append('#if %s\n\n' % receiver.condition)

    forward_declarations, headers = forward_declarations_and_headers_for_replies(receiver)

    result += headers
    result.append('\n')

    result.append(forward_declarations)
    result.append('\n')

    result.append('namespace Messages {\nnamespace %s {\n' % receiver.name)
    result.append('\n')
    result.append('\n'.join(filter(None, [message_to_reply_forward_declaration(x) for x in receiver.messages])))
    result.append('\n')
    result.append('} // namespace %s\n} // namespace Messages\n' % receiver.name)

    if receiver.condition:
        result.append('\n#endif // %s\n' % receiver.condition)

    return ''.join(result)


def generate_messages_header(receiver):
    result = []

    result.append(_license_header)

    result.append('#pragma once\n')
    result.append('\n')

    if receiver.condition:
        result.append('#if %s\n\n' % receiver.condition)

    forward_declarations, headers = forward_declarations_and_headers(receiver)

    result += headers
    result.append('\n')

    result.append(forward_declarations)
    result.append('\n')

    result.append('namespace Messages {\nnamespace %s {\n' % receiver.name)
    result.append('\n')
    result.append('static inline IPC::ReceiverName messageReceiverName()\n')
    result.append('{\n')
    result.append('    return IPC::ReceiverName::%s;\n' % receiver.name)
    result.append('}\n')
    result.append('\n')
    result.append('\n'.join([message_to_struct_declaration(receiver, x) for x in receiver.messages]))
    result.append('\n')
    result.append('} // namespace %s\n} // namespace Messages\n' % receiver.name)

    if receiver.condition:
        result.append('\n#endif // %s\n' % receiver.condition)

    return ''.join(result)


def handler_function(receiver, message):
    if message.name.find('URL') == 0:
        return '%s::%s' % (receiver.name, 'url' + message.name[3:])
    return '%s::%s' % (receiver.name, message.name[0].lower() + message.name[1:])


def async_message_statement(receiver, message):
    dispatch_function_args = ['decoder', 'this', '&%s' % handler_function(receiver, message)]

    dispatch_function = 'handleMessage'
    if message.has_attribute(ASYNC_ATTRIBUTE):
        dispatch_function += 'Async'
        dispatch_function_args.insert(0, 'connection')

    if message.has_attribute(WANTS_CONNECTION_ATTRIBUTE):
        if message.has_attribute(ASYNC_ATTRIBUTE):
            dispatch_function += 'WantsConnection'
        else:
            dispatch_function_args.insert(0, 'connection')

    result = []
    result.append('    if (decoder.messageName() == Messages::%s::%s::name()) {\n' % (receiver.name, message.name))
    result.append('        IPC::%s<Messages::%s::%s>(%s);\n' % (dispatch_function, receiver.name, message.name, ', '.join(dispatch_function_args)))
    result.append('        return;\n')
    result.append('    }\n')
    return surround_in_condition(''.join(result), message.condition)


def sync_message_statement(receiver, message):
    dispatch_function = 'handleMessage'
    if message.has_attribute(SYNCHRONOUS_ATTRIBUTE):
        dispatch_function += 'Synchronous'
        if message.has_attribute(WANTS_CONNECTION_ATTRIBUTE):
            dispatch_function += 'WantsConnection'
    if message.has_attribute(ASYNC_ATTRIBUTE):
        dispatch_function += 'Async'

    wants_connection = message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(WANTS_CONNECTION_ATTRIBUTE)

    result = []
    result.append('    if (decoder.messageName() == Messages::%s::%s::name()) {\n' % (receiver.name, message.name))
    result.append('        IPC::%s<Messages::%s::%s>(%sdecoder, %sreplyEncoder, this, &%s);\n' % (dispatch_function, receiver.name, message.name, 'connection, ' if wants_connection else '', '' if message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(ASYNC_ATTRIBUTE) else '*', handler_function(receiver, message)))
    result.append('        return;\n')
    result.append('    }\n')
    return surround_in_condition(''.join(result), message.condition)


def class_template_headers(template_string):
    template_string = template_string.strip()

    class_template_types = {
        'PurCFetcher::RectEdges': {'headers': ['<RectEdges.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'Expected': {'headers': ['<wtf/Expected.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'HashMap': {'headers': ['<wtf/HashMap.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'HashSet': {'headers': ['<wtf/HashSet.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'std::optional': {'headers': ['<optional>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'OptionSet': {'headers': ['<wtf/OptionSet.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'Vector': {'headers': ['<wtf/Vector.h>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
        'std::pair': {'headers': ['<utility>'], 'argument_coder_headers': ['"ArgumentCoders.h"']},
    }

    match = re.match('(?P<template_name>.+?)<(?P<parameter_string>.+)>', template_string)
    if not match:
        return {'header_infos': [], 'types': [template_string]}

    template_name = match.groupdict()['template_name']
    if template_name not in class_template_types:
        sys.stderr.write("Error: no class template type is defined for '%s'\n" % (template_string))
        sys.exit(1)

    header_infos = [class_template_types[template_name]]
    types = []

    for parameter in parser.split_parameters_string(match.groupdict()['parameter_string']):
        parameter_header_infos_and_types = class_template_headers(parameter)

        header_infos += parameter_header_infos_and_types['header_infos']
        types += parameter_header_infos_and_types['types']

    return {'header_infos': header_infos, 'types': types}


def argument_coder_headers_for_type(type):
    header_infos_and_types = class_template_headers(type)

    special_cases = {
        'String': '"ArgumentCoders.h"',
        'PurCFetcher::ScriptMessageHandlerHandle': '"WebScriptMessageHandler.h"',
    }

    headers = []
    for header_info in header_infos_and_types['header_infos']:
        headers += header_info['argument_coder_headers']

    for type in header_infos_and_types['types']:
        if type in special_cases:
            headers.append(special_cases[type])
            continue

        split = type.split('::')
        if len(split) < 2:
            continue
        if split[0] == 'PurCFetcher':
            headers.append('"WebCoreArgumentCoders.h"')

    return headers


def headers_for_type(type):
    header_infos_and_types = class_template_headers(type)

    special_cases = {
        'IPC::SharedBufferDataReference': ['"SharedBufferDataReference.h"', '"DataReference.h"'],
        'MachSendRight': ['<wtf/MachSendRight.h>'],
        'JSC::MessageLevel': ['<JavaScriptCore/ConsoleTypes.h>'],
        'JSC::MessageSource': ['<JavaScriptCore/ConsoleTypes.h>'],
        'Inspector::InspectorTargetType': ['<JavaScriptCore/InspectorTarget.h>'],
        'Inspector::FrontendChannel::ConnectionType': ['<JavaScriptCore/InspectorFrontendChannel.h>'],
        'MediaTime': ['<wtf/MediaTime.h>'],
        'MonotonicTime': ['<wtf/MonotonicTime.h>'],
        'Seconds': ['<wtf/Seconds.h>'],
        'WallTime': ['<wtf/WallTime.h>'],
        'String': ['<wtf/text/WTFString.h>'],
        'PAL::SessionID': ['"SessionID.h"'],
        'PurCFetcher::AutoplayEventFlags': ['<AutoplayEvent.h>'],
        'PurCFetcher::DOMPasteAccessResponse': ['<DOMPasteAccess.h>'],
        'PurCFetcher::DocumentOrWorkerIdentifier': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::DocumentEditingContextRequest': ['"DocumentEditingContext.h"'],
        'PurCFetcher::DragHandlingMethod': ['<DragActions.h>'],
        'PurCFetcher::DragOperation': ['<DragActions.h>'],
        'PurCFetcher::DragSourceAction': ['<DragActions.h>'],
        'PurCFetcher::ExceptionDetails': ['<JSDOMExceptionHandling.h>'],
        'PurCFetcher::FileChooserSettings': ['<FileChooser.h>'],
        'PurCFetcher::ShareDataWithParsedURL': ['<ShareData.h>'],
        'PurCFetcher::FontChanges': ['<FontAttributeChanges.h>'],
        'PurCFetcher::FrameLoadType': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::GenericCueData': ['<InbandGenericCue.h>'],
        'PurCFetcher::GrammarDetail': ['<TextCheckerClient.h>'],
        'PurCFetcher::HasInsecureContent': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::Highlight': ['<InspectorOverlay.h>'],
        'PurCFetcher::IncludeSecureCookies': ['<CookieJar.h>'],
        'PurCFetcher::IndexedDB::ObjectStoreOverwriteMode': ['<IndexedDB.h>'],
        'PurCFetcher::InputMode': ['<InputMode.h>'],
        'PurCFetcher::KeyframeValueList': ['<GraphicsLayer.h>'],
        'PurCFetcher::KeypressCommand': ['<KeyboardEvent.h>'],
        'PurCFetcher::LegacyCDMSessionClient::MediaKeyErrorCode': ['<LegacyCDMSession.h>'],
        'PurCFetcher::LockBackForwardList': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::MessagePortChannelProvider::HasActivity': ['<MessagePortChannelProvider.h>'],
        'PurCFetcher::MouseEventPolicy': ['<DocumentLoader.h>'],
        'PurCFetcher::NetworkTransactionInformation': ['<NetworkLoadInformation.h>'],
        'PurCFetcher::PasteboardCustomData': ['<Pasteboard.h>'],
        'PurCFetcher::PasteboardImage': ['<Pasteboard.h>'],
        'PurCFetcher::PasteboardURL': ['<Pasteboard.h>'],
        'PurCFetcher::PasteboardWebContent': ['<Pasteboard.h>'],
        'PurCFetcher::PaymentAuthorizationResult': ['<ApplePaySessionPaymentRequest.h>'],
        'PurCFetcher::PaymentMethodUpdate': ['<ApplePaySessionPaymentRequest.h>'],
        'PurCFetcher::PluginInfo': ['<PluginData.h>'],
        'PurCFetcher::PluginLoadClientPolicy': ['<PluginData.h>'],
        'PurCFetcher::PolicyAction': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::ShouldContinuePolicyCheck': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::PolicyCheckIdentifier': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::ProcessIdentifier': ['<ProcessIdentifier.h>'],
        'PurCFetcher::RecentSearch': ['<SearchPopupMenu.h>'],
        'PurCFetcher::RequestStorageAccessResult': ['<DocumentStorageAccess.h>'],
        'PurCFetcher::RouteSharingPolicy': ['<AudioSession.h>'],
        'PurCFetcher::SWServerConnectionIdentifier': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::SelectionDirection': ['<VisibleSelection.h>'],
        'PurCFetcher::ServiceWorkerJobIdentifier': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::ServiceWorkerOrClientData': ['<ServiceWorkerTypes.h>', '<ServiceWorkerClientData.h>', '<ServiceWorkerData.h>'],
        'PurCFetcher::ServiceWorkerOrClientIdentifier': ['<ServiceWorkerTypes.h>', '<ServiceWorkerClientIdentifier.h>'],
        'PurCFetcher::ServiceWorkerRegistrationIdentifier': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::ServiceWorkerRegistrationState': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::ServiceWorkerState': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::ShippingContactUpdate': ['<ApplePaySessionPaymentRequest.h>'],
        'PurCFetcher::ShippingMethodUpdate': ['<ApplePaySessionPaymentRequest.h>'],
        'PurCFetcher::ShouldAskITP': ['<NetworkStorageSession.h>'],
        'PurCFetcher::ShouldNotifyWhenResolved': ['<ServiceWorkerTypes.h>'],
        'PurCFetcher::ShouldSample': ['<DiagnosticLoggingClient.h>'],
        'PurCFetcher::StorageAccessPromptWasShown': ['<DocumentStorageAccess.h>'],
        'PurCFetcher::StorageAccessScope': ['<DocumentStorageAccess.h>'],
        'PurCFetcher::StorageAccessWasGranted': ['<DocumentStorageAccess.h>'],
        'PurCFetcher::SupportedPluginIdentifier': ['<PluginData.h>'],
        'PurCFetcher::SystemPreviewInfo': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::TextCheckingRequestData': ['<TextChecking.h>'],
        'PurCFetcher::TextCheckingResult': ['<TextCheckerClient.h>'],
        'PurCFetcher::TextCheckingType': ['<TextChecking.h>'],
        'PurCFetcher::TextIndicatorData': ['<TextIndicator.h>'],
        'PurCFetcher::ThirdPartyCookieBlockingMode': ['<NetworkStorageSession.h>'],
        'PurCFetcher::SameSiteStrictEnforcementEnabled': ['<NetworkStorageSession.h>'],
        'PurCFetcher::FirstPartyWebsiteDataRemovalMode': ['<NetworkStorageSession.h>'],
        'PurCFetcher::UsedLegacyTLS': ['<ResourceResponseBase.h>'],
        'PurCFetcher::ViewportAttributes': ['<ViewportArguments.h>'],
        'PurCFetcher::WebGLLoadPolicy': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::WillContinueLoading': ['<FrameLoaderTypes.h>'],
        'PurCFetcher::SelectionRect': ['"EditorState.h"'],
        'PurCFetcher::ActivityStateChangeID': ['"DrawingAreaInfo.h"'],
        'PurCFetcher::BackForwardListItemState': ['"SessionState.h"'],
        'PurCFetcher::ContentWorldIdentifier': ['"ContentWorldShared.h"'],
        'PurCFetcher::GestureRecognizerState': ['"GestureTypes.h"'],
        'PurCFetcher::GestureType': ['"GestureTypes.h"'],
        'PurCFetcher::LayerHostingContextID': ['"LayerHostingContext.h"'],
        'PurCFetcher::LayerHostingMode': ['"LayerTreeContext.h"'],
        'PurCFetcher::PageState': ['"SessionState.h"'],
        'PurCFetcher::PaymentSetupConfiguration': ['"PaymentSetupConfigurationPurCFetcher.h"'],
        'PurCFetcher::PaymentSetupFeatures': ['"ApplePayPaymentSetupFeaturesPurCFetcher.h"'],
        'PurCFetcher::PluginProcessType': ['"PluginProcessAttributes.h"'],
        'PurCFetcher::RespectSelectionAnchor': ['"GestureTypes.h"'],
        'PurCFetcher::SelectionFlags': ['"GestureTypes.h"'],
        'PurCFetcher::SelectionTouch': ['"GestureTypes.h"'],
        'PurCFetcher::WebGestureEvent': ['"WebEvent.h"'],
        'PurCFetcher::WebKeyboardEvent': ['"WebEvent.h"'],
        'PurCFetcher::WebMouseEvent': ['"WebEvent.h"'],
        'PurCFetcher::WebTouchEvent': ['"WebEvent.h"'],
        'PurCFetcher::WebWheelEvent': ['"WebEvent.h"'],
        'PurCFetcher::MediaEngineSupportParameters': ['<MediaPlayer.h>'],
        'PurCFetcher::ISOWebVTTCue': ['<ISOVTTCue.h>'],
        'struct PurCFetcher::Cookie': ['<Cookie.h>'],
        'struct PurCFetcher::ElementContext': ['<ElementContext.h>'],
        'struct PurCFetcher::WebUserScriptData': ['"WebUserContentControllerDataTypes.h"'],
        'struct PurCFetcher::WebUserStyleSheetData': ['"WebUserContentControllerDataTypes.h"'],
        'struct PurCFetcher::WebScriptMessageHandlerData': ['"WebUserContentControllerDataTypes.h"'],
        'webrtc::WebKitEncodedFrameInfo': ['<webrtc/sdk/PurCFetcher/WebKitEncoder.h>'],
        'webrtc::WebKitRTPFragmentationHeader': ['<webrtc/sdk/PurCFetcher/WebKitEncoder.h>'],
    }

    headers = []
    for header_info in header_infos_and_types['header_infos']:
        headers += header_info['headers']

    for type in header_infos_and_types['types']:
        if type in special_cases:
            headers += special_cases[type]
            continue

        # We assume that we must include a header for a type iff it has a scope
        # resolution operator (::).
        split = type.split('::')
        if len(split) < 2:
            continue

        if split[0] == 'PurCFetcher' or split[0] == 'IPC':
            headers.append('"%s.h"' % split[1])
        else:
            headers.append('<%s.h>' % split[1])

    return headers


def generate_message_handler(receiver):
    header_conditions = {
        '"%s"' % messages_header_filename(receiver): [None],
        '"HandleMessage.h"': [None],
        '"Decoder.h"': [None],
    }

    type_conditions = {}
    for parameter in receiver.iterparameters():
        if not parameter.type in type_conditions:
            type_conditions[parameter.type] = []

        if not parameter.condition in type_conditions[parameter.type]:
            type_conditions[parameter.type].append(parameter.condition)

    for parameter in receiver.iterparameters():
        type = parameter.type
        conditions = type_conditions[type]

        argument_encoder_headers = argument_coder_headers_for_type(type)
        if argument_encoder_headers:
            for header in argument_encoder_headers:
                if header not in header_conditions:
                    header_conditions[header] = []
                header_conditions[header].extend(conditions)

        type_headers = headers_for_type(type)
        for header in type_headers:
            if header not in header_conditions:
                header_conditions[header] = []
            header_conditions[header].extend(conditions)

    for message in receiver.messages:
        if message.reply_parameters is not None:
            for reply_parameter in message.reply_parameters:
                type = reply_parameter.type
                argument_encoder_headers = argument_coder_headers_for_type(type)
                if argument_encoder_headers:
                    for header in argument_encoder_headers:
                        if header not in header_conditions:
                            header_conditions[header] = []
                        header_conditions[header].append(message.condition)

                type_headers = headers_for_type(type)
                for header in type_headers:
                    if header not in header_conditions:
                        header_conditions[header] = []
                    header_conditions[header].append(message.condition)

    result = []

    result.append(_license_header)
    result.append('#include "config.h"\n')
    result.append('\n')

    if receiver.condition:
        result.append('#if %s\n\n' % receiver.condition)

    result.append('#include "%s.h"\n\n' % receiver.name)
    for header in sorted(header_conditions):
        if header_conditions[header] and not None in header_conditions[header]:
            result.append('#if %s\n' % ' || '.join(sorted(set(header_conditions[header]))))
            result += ['#include %s\n' % header]
            result.append('#endif\n')
        else:
            result += ['#include %s\n' % header]
    result.append('\n')

    delayed_or_async_messages = []
    for message in receiver.messages:
        if message.reply_parameters != None and (message.has_attribute(SYNCHRONOUS_ATTRIBUTE) or message.has_attribute(ASYNC_ATTRIBUTE)):
            delayed_or_async_messages.append(message)

    if delayed_or_async_messages:
        result.append('namespace Messages {\n\nnamespace %s {\n\n' % receiver.name)

        for message in delayed_or_async_messages:
            send_parameters = [(function_parameter_type(x.type, x.kind), x.name) for x in message.reply_parameters]

            if message.condition:
                result.append('#if %s\n\n' % message.condition)

            if message.has_attribute(ASYNC_ATTRIBUTE):
                move_parameters = message.name, ', '.join([move_type(x.type) for x in message.reply_parameters])
                result.append('void %s::callReply(IPC::Decoder& decoder, CompletionHandler<void(%s)>&& completionHandler)\n{\n' % move_parameters)
                result.append('    UNUSED_PARAM(decoder);\n')
                result.append('    UNUSED_PARAM(completionHandler);\n')
                for x in message.reply_parameters:
                    result.append('    std::optional<%s> %s;\n' % (x.type, x.name))
                    result.append('    decoder >> %s;\n' % x.name)
                    result.append('    if (!%s) {\n        ASSERT_NOT_REACHED();\n        cancelReply(WTFMove(completionHandler));\n        return;\n    }\n' % x.name)
                result.append('    completionHandler(')
                if len(message.reply_parameters):
                    result.append('WTFMove(*%s)' % ('), WTFMove(*'.join(x.name for x in message.reply_parameters)))
                result.append(');\n}\n\n')
                result.append('void %s::cancelReply(CompletionHandler<void(%s)>&& completionHandler)\n{\n    completionHandler(' % move_parameters)
                result.append(', '.join(['IPC::AsyncReplyError<' + x.type + '>::create()' for x in message.reply_parameters]))
                result.append(');\n}\n\n')

            result.append('void %s::send(std::unique_ptr<IPC::Encoder>&& encoder, IPC::Connection& connection' % (message.name))
            if len(send_parameters):
                result.append(', %s' % ', '.join([' '.join(x) for x in send_parameters]))
            result.append(')\n{\n')
            result += ['    *encoder << %s;\n' % x.name for x in message.reply_parameters]
            result.append('    connection.sendSyncReply(WTFMove(encoder));\n')
            result.append('}\n')
            result.append('\n')

            if message.condition:
                result.append('#endif\n\n')

        result.append('} // namespace %s\n\n} // namespace Messages\n\n' % receiver.name)

    result.append('namespace PurCFetcher {\n\n')

    async_messages = []
    sync_messages = []
    for message in receiver.messages:
        if message.reply_parameters is not None and not message.has_attribute(ASYNC_ATTRIBUTE):
            sync_messages.append(message)
        else:
            async_messages.append(message)

    if async_messages or receiver.has_attribute(WANTS_DISPATCH_MESSAGE_ATTRIBUTE):
        result.append('void %s::didReceive%sMessage(IPC::Connection& connection, IPC::Decoder& decoder)\n' % (receiver.name, receiver.name if receiver.has_attribute(LEGACY_RECEIVER_ATTRIBUTE) else ''))
        result.append('{\n')
        if not receiver.has_attribute(NOT_REFCOUNTED_RECEIVER_ATTRIBUTE):
            result.append('    auto protectedThis = makeRef(*this);\n')

        result += [async_message_statement(receiver, message) for message in async_messages]
        if receiver.has_attribute(WANTS_DISPATCH_MESSAGE_ATTRIBUTE):
            result.append('    if (dispatchMessage(connection, decoder))\n')
            result.append('        return;\n')
        if (receiver.superclass):
            result.append('    %s::didReceiveMessage(connection, decoder);\n' % (receiver.superclass))
        else:
            result.append('    UNUSED_PARAM(connection);\n')
            result.append('    UNUSED_PARAM(decoder);\n')
            result.append('    ASSERT_NOT_REACHED();\n')
        result.append('}\n')

    if sync_messages or receiver.has_attribute(WANTS_DISPATCH_MESSAGE_ATTRIBUTE):
        result.append('\n')
        result.append('void %s::didReceiveSync%sMessage(IPC::Connection& connection, IPC::Decoder& decoder, std::unique_ptr<IPC::Encoder>& replyEncoder)\n' % (receiver.name, receiver.name if receiver.has_attribute(LEGACY_RECEIVER_ATTRIBUTE) else ''))
        result.append('{\n')
        if not receiver.has_attribute(NOT_REFCOUNTED_RECEIVER_ATTRIBUTE):
            result.append('    auto protectedThis = makeRef(*this);\n')
        result += [sync_message_statement(receiver, message) for message in sync_messages]
        if receiver.has_attribute(WANTS_DISPATCH_MESSAGE_ATTRIBUTE):
            result.append('    if (dispatchSyncMessage(connection, decoder, replyEncoder))\n')
            result.append('        return;\n')
        result.append('    UNUSED_PARAM(connection);\n')
        result.append('    UNUSED_PARAM(decoder);\n')
        result.append('    UNUSED_PARAM(replyEncoder);\n')
        result.append('    ASSERT_NOT_REACHED();\n')
        result.append('}\n')

    result.append('\n} // namespace PurCFetcher\n\n')

    if receiver.condition:
        result.append('\n#endif // %s\n' % receiver.condition)

    return ''.join(result)


def generate_message_names_header(receivers):
    result = []
    result.append(_license_header)
    result.append('#pragma once\n')
    result.append('\n')
    result.append('#include <wtf/EnumTraits.h>\n')
    result.append('\n')
    result.append('namespace IPC {\n')
    result.append('\n')
    result.append('enum class ReceiverName : uint8_t {')

    enum_value = 1
    first_receiver = True
    for receiver in receivers:
        result.append('\n')
        result.append('    ')
        if not first_receiver:
            result.append(', ')
        first_receiver = False
        result.append('%s = %d' % (receiver.name, enum_value))
        enum_value = enum_value + 1
    result.append('\n    , IPC = %d' % enum_value)
    enum_value = enum_value + 1
    result.append('\n    , AsyncReply = %d' % enum_value)
    enum_value = enum_value + 1
    result.append('\n    , Invalid = %d' % enum_value)
    result.append('\n};\n')
    result.append('\n')
    result.append('enum class MessageName : uint16_t {')

    enum_value = 1
    first_message = True
    for receiver in receivers:
        for message in receiver.messages:
            result.append('\n')
            if message.condition:
                result.append('#if %s\n' % message.condition)
            result.append('    ')
            if not first_message:
                result.append(', ')
            first_message = False
            result.append('%s_%s = %d' % (receiver.name, message.name, enum_value))
            enum_value = enum_value + 1
            if message.has_attribute(ASYNC_ATTRIBUTE):
                result.append('\n    , %s_%sReply = %d' % (receiver.name, message.name, enum_value))
                enum_value = enum_value + 1
            if message.condition:
                result.append('\n#endif')
    result.append('\n    , WrappedAsyncMessageForTesting = %d' % enum_value)
    enum_value = enum_value + 1
    result.append('\n    , SyncMessageReply = %d' % enum_value)
    enum_value = enum_value + 1
    result.append('\n    , InitializeConnection = %d' % enum_value)
    enum_value = enum_value + 1
    result.append('\n    , LegacySessionState = %d' % enum_value)
    result.append('\n};\n')
    result.append('\n')
    result.append('ReceiverName receiverName(MessageName);\n')
    result.append('const char* description(MessageName);\n')
    result.append('bool isValidMessageName(MessageName);\n')
    result.append('\n')
    result.append('} // namespace IPC\n')
    result.append('\n')
    result.append('namespace WTF {\n')
    result.append('\n')
    result.append('template<>\n')
    result.append('class HasCustomIsValidEnum<IPC::MessageName> : public std::true_type { };\n')
    result.append('template<typename E, typename T, std::enable_if_t<std::is_same_v<E, IPC::MessageName>>* = nullptr>\n')
    result.append('bool isValidEnum(T messageName)\n')
    result.append('{\n')
    result.append('    static_assert(sizeof(T) == sizeof(E), "isValidEnum<IPC::MessageName> should only be called with 16-bit types");\n')
    result.append('    return IPC::isValidMessageName(static_cast<E>(messageName));\n')
    result.append('};\n')
    result.append('\n')
    result.append('} // namespace WTF\n')
    return ''.join(result)


def generate_message_names_implementation(receivers):
    result = []
    result.append(_license_header)
    result.append('#include "config.h"\n')
    result.append('#include "MessageNames.h"\n')
    result.append('\n')
    result.append('namespace IPC {\n')
    result.append('\n')
    result.append('const char* description(MessageName name)\n')
    result.append('{\n')
    result.append('    switch (name) {\n')
    for receiver in receivers:
        for message in receiver.messages:
            if message.condition:
                result.append('#if %s\n' % message.condition)
            result.append('    case MessageName::%s_%s:\n' % (receiver.name, message.name))
            result.append('        return "%s::%s";\n' % (receiver.name, message.name))
            if message.has_attribute(ASYNC_ATTRIBUTE):
                result.append('    case MessageName::%s_%sReply:\n' % (receiver.name, message.name))
                result.append('        return "%s::%sReply";\n' % (receiver.name, message.name))
            if message.condition:
                result.append('#endif\n')
    result.append('    case MessageName::WrappedAsyncMessageForTesting:\n')
    result.append('        return "IPC::WrappedAsyncMessageForTesting";\n')
    result.append('    case MessageName::SyncMessageReply:\n')
    result.append('        return "IPC::SyncMessageReply";\n')
    result.append('    case MessageName::InitializeConnection:\n')
    result.append('        return "IPC::InitializeConnection";\n')
    result.append('    case MessageName::LegacySessionState:\n')
    result.append('        return "IPC::LegacySessionState";\n')
    result.append('    }\n')
    result.append('    ASSERT_NOT_REACHED();\n')
    result.append('    return "<invalid message name>";\n')
    result.append('}\n')
    result.append('\n')
    result.append('ReceiverName receiverName(MessageName messageName)\n')
    result.append('{\n')
    result.append('    switch (messageName) {\n')
    for receiver in receivers:
        for message in receiver.messages:
            if message.condition:
                result.append('#if %s\n' % message.condition)
            result.append('    case MessageName::%s_%s:\n' % (receiver.name, message.name))
            if message.condition:
                result.append('#endif\n')
        result.append('        return ReceiverName::%s;\n' % receiver.name)
    for receiver in receivers:
        for message in receiver.messages:
            if message.has_attribute(ASYNC_ATTRIBUTE):
                if message.condition:
                    result.append('#if %s\n' % message.condition)
                result.append('    case MessageName::%s_%sReply:\n' % (receiver.name, message.name))
                if message.condition:
                    result.append('#endif\n')
    result.append('        return ReceiverName::AsyncReply;\n')
    result.append('    case MessageName::WrappedAsyncMessageForTesting:\n')
    result.append('    case MessageName::SyncMessageReply:\n')
    result.append('    case MessageName::InitializeConnection:\n')
    result.append('    case MessageName::LegacySessionState:\n')
    result.append('        return ReceiverName::IPC;\n')
    result.append('    }\n')
    result.append('    ASSERT_NOT_REACHED();\n')
    result.append('    return ReceiverName::Invalid;\n')
    result.append('}\n')
    result.append('\n')
    result.append('bool isValidMessageName(MessageName messageName)\n')
    result.append('{\n')
    for receiver in receivers:
        for message in receiver.messages:
            if message.condition:
                result.append('#if %s\n' % message.condition)
            result.append('    if (messageName == IPC::MessageName::%s_%s)\n' % (receiver.name, message.name))
            result.append('        return true;\n')
            if message.has_attribute(ASYNC_ATTRIBUTE):
                result.append('    if (messageName == IPC::MessageName::%s_%sReply)\n' % (receiver.name, message.name))
                result.append('        return true;\n')
            if message.condition:
                result.append('#endif\n')
    result.append('    if (messageName == IPC::MessageName::WrappedAsyncMessageForTesting)\n')
    result.append('        return true;\n')
    result.append('    if (messageName == IPC::MessageName::SyncMessageReply)\n')
    result.append('        return true;\n')
    result.append('    if (messageName == IPC::MessageName::InitializeConnection)\n')
    result.append('        return true;\n')
    result.append('    if (messageName == IPC::MessageName::LegacySessionState)\n')
    result.append('        return true;\n')
    result.append('    return false;\n')
    result.append('};\n')
    result.append('\n')
    result.append('} // namespace IPC\n')
    return ''.join(result)
