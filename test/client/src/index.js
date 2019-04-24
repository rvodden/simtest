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
        this.setState({ value: ! this.state.value });
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
        super(props);
    }

    componentDidMount() {
        this.connection = new WebSocket('ws://localhost:7681','websocket-protocol');
        this.connection.onMessage = data => {
            this.led.handleData(data)
        }
    }

    sendMessage(message, name) {
        var jsonMessage = { name: name, message: {value: message}  };
        this.connection.send(JSON.stringify(jsonMessage));
    }


    render() {
        return (
            <div>
                <svg version="1.1" width={100} height={100}>
                    <Led
                        value={true}
                        ref={this.led}
                        name="LED"
                    />
                    <Button
                        sendMessage={(message) => this.sendMessage(message, "button")}
                        name="BUTTON"
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
