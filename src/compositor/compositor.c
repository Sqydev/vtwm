#include "../coredata.h"

#include "./compositor.h"
#include "../events/events.h"
#include "../types.h"

#include <stdio.h>
#include <stdlib.h>

#include <wayland-server-core.h>

#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_linux_dmabuf_v1.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_shm.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_fractional_scale_v1.h>
#include <wlr/types/wlr_presentation_time.h>
#include <wlr/types/wlr_viewporter.h>
#include <wlr/types/wlr_xdg_output_v1.h>

int InitCompositor(Compositor* compositor) {
	compositor->display = wl_display_create();
	if(!compositor->display) {
		fprintf(stderr, "Failed to create Wayland display\n");
		return -1;
	}

	compositor->backend = wlr_backend_autocreate(wl_display_get_event_loop(compositor->display), NULL);
	if(!compositor->backend) {
		fprintf(stderr, "Failed to create wlroots backend\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->renderer = wlr_renderer_autocreate(compositor->backend);
	if(!compositor->renderer) {
		fprintf(stderr, "Failed to create wlroots renderer\n");
		FreeCompositor(compositor);

		return -1;
	}

	if(!wlr_renderer_init_wl_display(compositor->renderer, compositor->display)) {
		fprintf(stderr, "Failed to initialize wl_display rendering\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->allocator = wlr_allocator_autocreate(compositor->backend, compositor->renderer);
	if(!compositor->allocator) {
		fprintf(stderr, "Failed to create wlroots allocator\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->compositor = wlr_compositor_create(compositor->display, 5, compositor->renderer);
	if(!compositor->compositor) {
		fprintf(stderr, "Failed to create wl_compositor\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->scene = wlr_scene_create();
	if(!compositor->scene) {
		fprintf(stderr, "Failed to create scene\n");
		FreeCompositor(compositor);
	
		return -1;
	}

	compositor->outputLayout = wlr_output_layout_create(compositor->display);
	if(!compositor->outputLayout) {
		fprintf(stderr, "Failed to create output layout\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->sceneLayout = wlr_scene_attach_output_layout(compositor->scene, compositor->outputLayout);
	if(!compositor->sceneLayout) {
		fprintf(stderr, "Failed to attach output layout to scene\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->subcompositor = wlr_subcompositor_create(compositor->display);
	if(!compositor->subcompositor) {
		fprintf(stderr, "Failed to create wl_subcompositor\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->shm = wlr_shm_create_with_renderer(compositor->display, 1, compositor->renderer);
	if(!compositor->shm) {
		fprintf(stderr, "Failed to create wl_shm\n");
		FreeCompositor(compositor);

		return -1;
	}

	compositor->xdgShell = wlr_xdg_shell_create(compositor->display, 3);
	if(!compositor->xdgShell) {
		fprintf(stderr, "Failed to create xdg-shell\n");
		FreeCompositor(compositor);

		return -1;
	}

	if(compositor->renderer->render_buffer_caps & WLR_BUFFER_CAP_DMABUF) {
		compositor->linuxDmaBuf = wlr_linux_dmabuf_v1_create_with_renderer(compositor->display, 4, compositor->renderer);
	}

	if(!compositor->linuxDmaBuf) {
		fprintf(stderr, "Warning: linux-dmabuf-v1 disabled (no importable DMA-BUF path)\n");
	}
		
	wlr_viewporter_create(compositor->display);
	wlr_presentation_create(compositor->display, compositor->backend, 2);
	wlr_fractional_scale_manager_v1_create(compositor->display, 1);
	wlr_xdg_output_manager_v1_create(compositor->display, compositor->outputLayout);

	compositor->dataDeviceManager = wlr_data_device_manager_create(compositor->display);

	if(!compositor->dataDeviceManager) {
		fprintf(stderr, "Failed to create wl_data_device_manager\n");
		FreeCompositor(compositor);

		return -1;
	}
	
	InitEvents(&DATA.compositor, &DATA.events);

	compositor->socket = wl_display_add_socket_auto(compositor->display);
	if(!compositor->socket) {
		fprintf(stderr, "Failed to create Wayland socket\n");
		FreeCompositor(compositor);

		return -1;
	}

	printf("Wayland socket: %s\n", compositor->socket);
	
	// CHildren will have good socket autopmaticly
	setenv("WAYLAND_DISPLAY", compositor->socket, true);

	// Make also it configurable. Like. The inicial value
	DATA.workspaceManager.wdirsCount = 1;
	DATA.workspaceManager.currentWdirIdx = 0;
	DATA.workspaceManager.currLayer = LAYER_NORMAL;
	DATA.workspaceManager.wdir = malloc(DATA.workspaceManager.wdirsCount * sizeof(WorkspaceDir));
	if(!DATA.workspaceManager.wdir) {
		fprintf(stderr, "Malloc exploded on workspace dirs :)\n");
		FreeCompositor(compositor);

		return -1;
	}
	
	WorkspaceDir* direr = &DATA.workspaceManager.wdir[DATA.workspaceManager.currentWdirIdx];
	direr->workspacesCount = 1;
	direr->currentWorkspaceIdx = 0;
	direr->workspaces[LAYER_TTY] = malloc(direr->workspacesCount * sizeof(Workspace));
	direr->workspaces[LAYER_NORMAL] = malloc(direr->workspacesCount * sizeof(Workspace));
	direr->workspaces[LAYER_SUPER] = malloc(direr->workspacesCount * sizeof(Workspace));

	direr->workspaces[LAYER_TTY]->windowsCount = 0;
	direr->workspaces[LAYER_TTY]->windows = NULL;
	direr->workspaces[LAYER_NORMAL]->windowsCount = 0;
	direr->workspaces[LAYER_NORMAL]->windows = NULL;
	direr->workspaces[LAYER_SUPER]->windowsCount = 0;
	direr->workspaces[LAYER_SUPER]->windows = NULL;

	return 0;
}

int RunCompositor(Compositor* compositor) {
	if(!wlr_backend_start(compositor->backend)) {
		fprintf(stderr, "Failed to start wlroots backend\n");

		return -1;
	}

	wl_display_run(compositor->display);

	return 0;
}

void FreeCompositor(Compositor* compositor) {
	WorkspaceDir* direr = &DATA.workspaceManager.wdir[DATA.workspaceManager.currentWdirIdx];

	direr->workspaces[LAYER_TTY]->windowsCount = 0;
	direr->workspaces[LAYER_NORMAL]->windowsCount = 0;
	direr->workspaces[LAYER_SUPER]->windowsCount = 0;

	if(direr->workspaces[LAYER_TTY]->windows) {
		direr->workspaces[LAYER_TTY]->windows = NULL;
	}
	if(direr->workspaces[LAYER_NORMAL]->windows) {
		direr->workspaces[LAYER_NORMAL]->windows = NULL;
	}
	if(direr->workspaces[LAYER_SUPER]->windows) {
		direr->workspaces[LAYER_SUPER]->windows = NULL;
	}

	direr->workspacesCount = 0;
	direr->currentWorkspaceIdx = 0;
	if(direr->workspaces[LAYER_TTY]) {
		free(direr->workspaces[LAYER_TTY]);
	}
	if(direr->workspaces[LAYER_NORMAL]) {
		free(direr->workspaces[LAYER_NORMAL]);
	}
	if(direr->workspaces[LAYER_SUPER]) {
		free(direr->workspaces[LAYER_SUPER]);
	}

	DATA.workspaceManager.wdirsCount = 0;
	DATA.workspaceManager.currentWdirIdx = 0;
	DATA.workspaceManager.currLayer = LAYER_NORMAL;
	if(DATA.workspaceManager.wdir) {
		free(DATA.workspaceManager.wdir);
	}
	
	if(compositor->outputLayout) {
		wlr_output_layout_destroy(compositor->outputLayout);
		compositor->outputLayout = NULL;
	}

	if(compositor->allocator) {
		wlr_allocator_destroy(compositor->allocator);
		compositor->allocator = NULL;
	}

	if(compositor->renderer) {
		wlr_renderer_destroy(compositor->renderer);
		compositor->renderer = NULL;
	}

	if(compositor->backend) {
		wlr_backend_destroy(compositor->backend);
		compositor->backend = NULL;
	}

	if(compositor->display) {
		wl_display_destroy_clients(compositor->display);
		wl_display_destroy(compositor->display);

		compositor->display = NULL;
	}
}
