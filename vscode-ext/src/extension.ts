import * as vscode from 'vscode';
import { LanguageClient, Executable, LanguageClientOptions } from 'vscode-languageclient'

class Debugger implements vscode.DebugAdapterDescriptorFactory {
	path: string

	constructor(path: string) {
		this.path = path
	}

	createDebugAdapterDescriptor(session: vscode.DebugSession, executable: vscode.DebugAdapterExecutable | undefined): vscode.ProviderResult<vscode.DebugAdapterDescriptor> {
		if (executable) {
			return executable
		}

		return new vscode.DebugAdapterExecutable(this.path, ['--debugger'])
	}
}

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

	context.subscriptions.push(vscode.debug.registerDebugAdapterDescriptorFactory('duality', new Debugger(path)))
}

export function deactivate() {
	if (client) {
		client.stop()
	}
}
