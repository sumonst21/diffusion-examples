/**
 * Copyright © 2018, 2021 Push Technology Ltd.
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
 *
 * This example is written in C99. Please use an appropriate C99 capable compiler
 *
 * @author Push Technology Limited
 * @since 6.2
 */

/*
 * This example shows how a request can be sent to another client via
 * a request path endpoint. The session ID of the target client must be
 * known.
 */

#include <stdio.h>

#ifndef WIN32
#include <unistd.h>
#else
#define sleep(x) Sleep(1000 * x)
#endif

#include "diffusion.h"
#include "args.h"

char *response;

ARG_OPTS_T arg_opts[] = {
        ARG_OPTS_HELP,
        {'u', "url", "Diffusion server URL", ARG_OPTIONAL, ARG_HAS_VALUE, "ws://localhost:8080"},
        {'p', "principal", "Principal (username) for the connection", ARG_OPTIONAL, ARG_HAS_VALUE, "control"},
        {'c', "credentials", "Credentials (password) for the connection", ARG_OPTIONAL, ARG_HAS_VALUE, "password"},
        {'t', "request_path", "Request path", ARG_REQUIRED, ARG_HAS_VALUE, "echo"},
        {'d', "request", "Request to send", ARG_REQUIRED, ARG_HAS_VALUE, "hello client request!"},
        {'r', "response", "Response to send", ARG_REQUIRED, ARG_HAS_VALUE, "hello client response!"},
        END_OF_ARG_OPTS
};

static int
on_request(SESSION_T *session, const char *request_path, DIFFUSION_DATATYPE request_datatype,
           const DIFFUSION_VALUE_T *request, const DIFFUSION_RESPONDER_HANDLE_T *handle, void *context)
{

        char *request_val;
        read_diffusion_string_value(request, &request_val, NULL);
        printf("Request received: %s\n", request_val);
        free(request_val);

        BUF_T *response_buf = buf_create();
        write_diffusion_string_value(response, response_buf);
        diffusion_respond_to_request(session, handle, response_buf, NULL);
        buf_free(response_buf);

        return HANDLER_SUCCESS;
}

static int
on_response(DIFFUSION_DATATYPE response_datatype, const DIFFUSION_VALUE_T *response, void *context)
{
        char *response_val;
        read_diffusion_string_value(response, &response_val, NULL);
        printf("Response received: %s\n\n", response_val);
        free(response_val);

        return HANDLER_SUCCESS;
}

int
main(int argc, char **argv)
{
        /*
         * Standard command-line parsing.
         */
        HASH_T *options = parse_cmdline(argc, argv, arg_opts);
        if(options == NULL || hash_get(options, "help") != NULL) {
                show_usage(argc, argv, arg_opts);
                return EXIT_FAILURE;
        }

        char *url = hash_get(options, "url");

        const char *principal = hash_get(options, "principal");
        CREDENTIALS_T *credentials = NULL;

        const char *password = hash_get(options, "credentials");
        if(password != NULL) {
                credentials = credentials_create_password(password);
        }

        char *request_path = hash_get(options, "request_path");

        /*
         * Create a session with Diffusion.
         */
        SESSION_T *session = NULL;
        DIFFUSION_ERROR_T error = { 0 };
        session = session_create(url, principal, credentials, NULL, NULL, &error);
        if(session == NULL) {
                fprintf(stderr, "TEST: Failed to create session\n");
                fprintf(stderr, "ERR : %s\n", error.message);
                return EXIT_FAILURE;
        }

        /*
         * Create a payload.
         */
        char *request_data = hash_get(options, "request");
        response = hash_get(options, "response");

        BUF_T *request = buf_create();

        write_diffusion_string_value(request_data, request);

        DIFFUSION_REQUEST_STREAM_T request_stream = {
                .on_request = on_request
        };

        set_request_stream(session, request_path, DATATYPE_STRING, DATATYPE_STRING, &request_stream);

        SEND_REQUEST_TO_SESSION_PARAMS_T params = {
                .recipient_session = session->id,
                .path = request_path,
                .request = request,
                .request_datatype = DATATYPE_STRING,
                .response_datatype = DATATYPE_STRING,
                .on_response = on_response
        };

        int counter = 1;

        while (counter <= 120) {
                printf("Sending request.. #%d\n", counter);
                send_request_to_session(session, params);
                sleep(1);
                ++counter;
        }

        session_close(session, NULL);
        session_free(session);
        buf_free(request);
        credentials_free(credentials);
        hash_free(options, NULL, free);

        return EXIT_SUCCESS;
}