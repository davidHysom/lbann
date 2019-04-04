"""Helper functions to add common command-line arguments."""
import argparse
import lbann.optimizer

def add_scheduler_arguments(parser):
    """Add command-line arguments for common scheduler settings.

    Adds the following options: `--nodes`, `--procs-per-node`,
    `--partition`, `--account`, `--time-limit`. The caller is
    responsible for using them.

    Args:
        parser (argparse.ArgumentParser): command-line argument
            parser.

    """
    if not isinstance(parser, argparse.ArgumentParser):
        raise TypeError('expected an argparse.ArgumentParser')
    parser.add_argument(
        '--nodes', action='store', type=int,
        help='number of compute nodes', metavar='NUM')
    parser.add_argument(
        '--procs-per-node', action='store', type=int,
        help='number of processes per compute node', metavar='NUM')
    parser.add_argument(
        '--partition', action='store', type=str,
        help='scheduler partition', metavar='NAME')
    parser.add_argument(
        '--account', action='store', type=str,
        help='scheduler account', metavar='NAME')
    parser.add_argument(
        '--time-limit', action='store', type=int,
        help='time limit (in minutes)', metavar='MIN')

def add_optimizer_arguments(parser):
    """Add command-line arguments for optimizers.

    Adds the following options: `--optimizer`,
    `--optimizer-learning-rate`. The parsed arguments
    (e.g. `parser.parse_args()`) are the input for `create_optimizer`.

    Args:
        parser (argparse.ArgumentParser): command-line argument
            parser.

    """
    if not isinstance(parser, argparse.ArgumentParser):
        raise TypeError('expected an argparse.ArgumentParser')
    parser.add_argument(
        '--optimizer', action='store', default='momentum', type=str,
        choices=('momentum', 'sgd', 'adam', 'adagrad', 'rmsprop'),
        help='optimizer (default: momentum)')
    parser.add_argument(
        '--optimizer-learning-rate',
        action='store', default=0.01, type=float,
        help='optimizer learning rate (default: 0.01)', metavar='VAL')

def create_optimizer(args):
    """Create optimizer from command-line arguments.

    The parsed arguments must be generated by an
    `argparse.ArgumentParser` that has been processed by
    `add_optimizer_arguments`.

    Args:
        args (Namespace): A namespace returned by
           `argparse.ArgumentParser.parse_args`.

    Return:
        lbann.optimizer.Optimizer

    """

    # Get parsed command-line arguments
    try:
        opt = args.optimizer
        lr = args.optimizer_learning_rate
    except AttributeError:
        raise ValueError('parsed arguments have not been processed '
                         'by `add_optimizer_arguments`')

    # Create optimizer
    if opt == 'momentum':
        return lbann.optimizer.SGD(learn_rate=lr, momentum=0.9)
    elif opt == 'sgd':
        return lbann.optimizer.SGD(learn_rate=lr)
    elif opt == 'adam':
        return lbann.optimizer.Adam(learn_rate=lr, beta1=0.9, beta2=0.99, eps=1e-8)
    elif opt == 'adagrad':
        return lbann.optimizer.AdaGrad(learn_rate=lr, eps=1e-8)
    elif opt == 'rmsprop':
        return lbann.optimizer.RMSprop(learn_rate=lr, decay_rate=0.99, eps=1e-8)
    else:
        raise ValueError('invalid optimizer type ({})'.format(opt))