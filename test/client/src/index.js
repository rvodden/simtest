import React from 'react';
import ReactDom from 'react-dom'

class Led extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            value: this.props.value,
        }
    }

    handleData(data) {
        console.log(`Received data: ${data.data}`);
        var event = JSON.parse(data.data);
        this.setState({ value: event.message.value });
    }

    render() {
        return (
            <circle cx={30} cy={50} r={10} fill={this.state.value ? "red" : "black"} stroke="black" />
        )
    }

}

class Button extends React.Component {
    constructor(props) {
        super(props);
        this.sendMessage = this.props.sendMessage
    }

    render() {
        return (
            <circle cx={60} cy={50} r={10} fill="grey" stroke="black"
                onMouseDown = { () => this.sendMessage(true)  }
                onMouseUp   = { () => this.sendMessage(false) }
            />
        )
    }
}

class Board extends React.Component {
    constructor(props) {
        super(props)
        this.setLed = element => {
            this.led = element;
        }
    }

    componentDidMount() {
        this.connection = new WebSocket('ws://localhost:7681','websocket-protocol');
        this.connection.onmessage = data => {
            console.log(`Received message: ${JSON.stringify(data)}`)
            this.led.handleData(data)
        };
    }

    sendMessage(message, name) {
        var jsonMessage = { name: name, message: {value: message}  };
        console.log(`Sending message: ${JSON.stringify(jsonMessage)}`);
        this.connection.send(JSON.stringify(jsonMessage));
    }

    render() {
        return (
            <div>
                <svg version="1.1" width={100} height={100}>
                    <Led
                        value={true}
                        ref={this.setLed}
                        name="led"
                    />
                    <Button
                        sendMessage={(message) => this.sendMessage(message, "button")}
                        name="button"
                    />
                </svg>
            </div>
        )
    }
}

ReactDom.render(
    <Board />,
    document.getElementById('root')
);
