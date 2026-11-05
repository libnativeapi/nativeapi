module github.com/libnativeapi/nativeapi/examples/go_mygo_display_example

go 1.27.3

require (
	github.com/egoist/mygo v0.7.4
	github.com/libnativeapi/nativeapi/bindings/go v0.0.0
)

require (
	github.com/ebitengine/purego v0.11.1 // indirect
	github.com/go-text/typesetting v0.3.5 // indirect
	golang.org/x/image v0.46.0 // indirect
	golang.org/x/net v0.59.0 // indirect
	golang.org/x/text v0.43.0 // indirect
)

replace github.com/libnativeapi/nativeapi/bindings/go => ../../bindings/go
