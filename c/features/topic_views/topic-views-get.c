/**
 * Copyright © 2022 Push Technology Ltd.
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
 * @since 6.8
 */

/*
 * This example creates multiple topics and corresponding topic views.
 * The topic views are listed and we retrieve a topic view.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef WIN32
        #include <unistd.h>
#else
        #define sleep(x) Sleep(1000 * x)
#endif

#include "diffusion.h"
#include "args.h"
#include "conversation.h"


ARG_OPTS_T arg_opts[] = {
        ARG_OPTS_HELP,
        {'u', "url", "Diffusion server URL", ARG_OPTIONAL, ARG_HAS_VALUE, "ws://localhost:8080"},
        {'p', "principal", "Principal (username) for the connection", ARG_OPTIONAL, ARG_HAS_VALUE, "control"},
        {'c', "credentials", "Credentials (password) for the connection", ARG_OPTIONAL, ARG_HAS_VALUE, "password"},
        {'t', "topic", "Topic name to create and update", ARG_OPTIONAL, ARG_HAS_VALUE, "source"},
        END_OF_ARG_OPTS
};

// Forward declaration
static void print_topic_view(DIFFUSION_TOPIC_VIEW_T *topic_view);


// Handlers for add topic feature.
static int on_topic_added_with_specification(
        SESSION_T *session,
        TOPIC_ADD_RESULT_CODE result_code,
        void *context)
{
        printf("Added topic \"%s\"\n", (const char *)context);
        return HANDLER_SUCCESS;
}


static int on_topic_add_failed_with_specification(
        SESSION_T *session,
        TOPIC_ADD_FAIL_RESULT_CODE result_code,
        const DIFFUSION_ERROR_T *error,
        void *context)
{
        printf("Failed to add topic \"%s\" (%d)\n", (const char *)context, result_code);
        return HANDLER_SUCCESS;
}


static int on_topic_add_discard(
        SESSION_T *session,
        void *context)
{
        printf("Topic add discarded\n");
        return HANDLER_SUCCESS;
}


static ADD_TOPIC_CALLBACK_T create_topic_callback(const char *topic_name)
{
        ADD_TOPIC_CALLBACK_T callback = {
                .on_topic_added_with_specification = on_topic_added_with_specification,
                .on_topic_add_failed_with_specification = on_topic_add_failed_with_specification,
                .on_discard = on_topic_add_discard,
                .context = (char *)topic_name
        };

        return callback;
}


static int on_topic_view_created(
        const DIFFUSION_TOPIC_VIEW_T *topic_view,
        void *context)
{
        char *view_name = diffusion_topic_view_get_name(topic_view);
        char *spec = diffusion_topic_view_get_specification(topic_view);

        printf("Topic view \"%s\" created with specification \"%s\"\n", view_name, spec);

        free(view_name);
        free(spec);

        return HANDLER_SUCCESS;
}


static int on_error(SESSION_T *session, const DIFFUSION_ERROR_T *error)
{
        printf("Error: %s\n", error->message);
        return HANDLER_SUCCESS;
}


// Handlers for listing Topic views
static int on_topic_views_list(
        const LIST_T *topic_views,
        void *context)
{
        int size = list_get_size(topic_views);

        printf("Total topic views: %d\n", size);
        for (int i = 0; i < size; i++) {
                DIFFUSION_TOPIC_VIEW_T *topic_view = list_get_data_indexed(topic_views, i);
                print_topic_view(topic_view);
        }
        return HANDLER_SUCCESS;
}


static int on_error_list(
        SESSION_T *session,
        const DIFFUSION_ERROR_T *error)
{
        printf("An error has occured while listing Topic Views: (%d) %s\n", error->code, error->message);

        return HANDLER_SUCCESS;
}


// Handlers for retrieving the topic view information
static int on_topic_view_get (const DIFFUSION_TOPIC_VIEW_T *topic_view, void *context)
{
        printf("Received a topic view.\n");
        print_topic_view((DIFFUSION_TOPIC_VIEW_T *) topic_view);

        return HANDLER_SUCCESS;
}


static int on_error_get(
        SESSION_T *session,
        const DIFFUSION_ERROR_T *error)
{
        printf("An error has occured while retrieving a Topic View: (%d) %s\n", error->code, error->message);

        return HANDLER_SUCCESS;
}


// Helper functions to create topics and topic views, list, get and print topic views
static void print_topic_view(DIFFUSION_TOPIC_VIEW_T *topic_view)
{
        char *view_name = diffusion_topic_view_get_name(topic_view);
        char *view_specification = diffusion_topic_view_get_specification(topic_view);
        SET_T *view_roles = diffusion_topic_view_get_roles(topic_view);

        printf("%s: [%s] [", view_name, view_specification);
        char **values = (char **) set_values(view_roles);
        for(char **value = values; *value != NULL; value++) {
                printf("%s ", *value);
        }
        printf("]\n");

        free(values);
        set_free(view_roles);
        free(view_specification);
        free(view_name);
}


static void create_topic_and_topic_view(
        SESSION_T *session,
        char *root_topic_path,
        char *topic_name,
        char *view_name)
{
        char *topic_path = calloc(strlen(root_topic_path) + strlen(topic_name) + 2, sizeof(char));
        sprintf(topic_path, "%s/%s", root_topic_path, topic_name);

        char *topic_view_path = calloc(strlen(view_name) + 7, sizeof(char));
        sprintf(topic_view_path, "views/%s", view_name);

        ADD_TOPIC_CALLBACK_T callback = create_topic_callback(topic_path);
        TOPIC_SPECIFICATION_T *spec = topic_specification_init(TOPIC_TYPE_STRING);

        // Create the source topic.
        add_topic_from_specification(session, topic_path, spec, callback);

        // Sleep for a while
        sleep(5);

        BUF_T *buf = buf_create();
        buf_sprintf(buf, "map %s to %s", topic_path, topic_view_path);

        char *topic_view_spec = buf_as_string(buf);

        DIFFUSION_CREATE_TOPIC_VIEW_PARAMS_T topic_view_params = {
                .view = view_name,
                .specification = topic_view_spec,
                .on_topic_view_created = on_topic_view_created,
                .on_error = on_error
        };

        // Send the request to create the topic view.
        diffusion_topic_views_create_topic_view(session, topic_view_params, NULL);

        // Sleep for a while
        sleep(5);

        // Free resources.
        free(topic_view_spec);
        buf_free(buf);
        topic_specification_free(spec);
        free(topic_view_path);
        free(topic_path);
}


static void list_topic_views(SESSION_T *session)
{
       DIFFUSION_TOPIC_VIEWS_LIST_PARAMS_T params_list = {
               .on_topic_views_list = on_topic_views_list,
               .on_error = on_error_list
       };
       diffusion_topic_views_list_topic_views(session, params_list, NULL);

       // Sleep for a while
       sleep(5);
}


static void get_topic_view(
        SESSION_T *session,
        char *view_name)
{
        DIFFUSION_GET_TOPIC_VIEW_PARAMS_T params = {
                .name = view_name,
                .on_topic_view = on_topic_view_get,
                .on_error = on_error_get,
        };

        // Send the request to retrieve the topic view.
        diffusion_topic_views_get_topic_view(session, params, NULL);

        // Sleep for a while
        sleep(5);
}


// Program entry point.
int main(int argc, char** argv)
{
        // Standard command-line parsing.
        HASH_T *options = parse_cmdline(argc, argv, arg_opts);
        if(options == NULL || hash_get(options, "help") != NULL) {
                show_usage(argc, argv, arg_opts);
                return EXIT_FAILURE;
        }

        const char *url = hash_get(options, "url");
        const char *principal = hash_get(options, "principal");
        const char *password = hash_get(options, "credentials");
        const char *topic_name = hash_get(options, "topic");

        CREDENTIALS_T *credentials = NULL;
        if(password != NULL) {
                credentials = credentials_create_password(password);
        }

        // Create a session with the Diffusion server.
        SESSION_T *session;
        DIFFUSION_ERROR_T error = { 0 };
        session = session_create(url, principal, credentials, NULL, NULL, &error);
        if(session == NULL) {
                fprintf(stderr, "Failed to create session\n");
                fprintf(stderr, "%s\n", error.message);
                return EXIT_FAILURE;
        }

        // Create multiple topics and corresponding topic views
         create_topic_and_topic_view(session, (char *) topic_name, "topic_path_example_1", "view_1");
         create_topic_and_topic_view(session, (char *) topic_name, "topic_path_example_2", "view_2");
         create_topic_and_topic_view(session, (char *) topic_name, "topic_path_example_3", "view_3");
         create_topic_and_topic_view(session, (char *) topic_name, "topic_path_example_4", "view_4");

        // List the topic views before removal
        list_topic_views(session);

        // Get topic view details
        get_topic_view(session, "view_1");
        get_topic_view(session, "view_2");
        get_topic_view(session, "view_3");
        get_topic_view(session, "view_4");

        // Close session and free resources.
        session_close(session, NULL);
        session_free(session);

        credentials_free(credentials);
        hash_free(options, NULL, free);

        return EXIT_SUCCESS;
}
