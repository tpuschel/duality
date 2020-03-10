import * as vscode from 'vscode';
import { LanguageClient, Executable, LanguageClientOptions } from 'vscode-languageclient'

let client: LanguageClient

export function activate(context: vscode.ExtensionContext) {
	let path = vscode.workspace.getConfiguration('duality').get('path') as string
	if (!path) {
		path = "duality"
	}

	const server: Executable = {
		command: path,
		args: ['--server']
	}

	const clientOptions: LanguageClientOptions = {
		documentSelector: [{ language: 'duality' }]
	}

	client = new LanguageClient('Duality Language Server', server, clientOptions)

	client.start()
}

export function deactivate() {
	if (client) {
		client.stop()
	}
}
