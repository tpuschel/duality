import * as vscode from 'vscode';
import { LanguageClient, Executable, LanguageClientOptions } from 'vscode-languageclient'

let client: LanguageClient

export function activate(context: vscode.ExtensionContext) {
	const server: Executable = {
		command: 'duality',
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
