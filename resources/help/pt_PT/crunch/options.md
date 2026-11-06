# Alterar as definições do JeffCrunch
<!-- keywords: edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Após premir Enter, mover a seleção** indica para onde segue a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir a conclusão de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto e apenas um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Retrocesso** para o retirar. Este preenchimento considera apenas o bloco de células ao lado daquela em que está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastamento do limite da seleção. Desative-o se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero nas células que têm o valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando estiver desativado, uma célula com 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha fica guardada com a folha.
- **Colocar uma vírgula decimal nos números inteiros introduzidos** chama-se casas decimais automáticas. Defina **Casas** como 2 e, ao escrever 1234, o valor introduzido é 12,34. Defina-o como -2 e, ao escrever 5, o valor introduzido é 500. Um número escrito com vírgula decimal é introduzido tal como o escreveu.

- **Permitir a edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou F2 vai para a barra de fórmulas, e a célula só mostra o resultado quando termina a entrada.

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** é a dimensão com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de todas as colunas desta folha que não têm largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Opções de privacidade

- **Retirar o meu nome e a empresa das propriedades de um livro sempre que este for guardado** limpa o autor, quem o guardou por último, a empresa e o gestor das propriedades do livro ao guardar. As propriedades são as informações que um ficheiro guarda sobre si próprio.

## Estatísticas de utilização

**Enviar estatísticas de utilização anónimas** permite partilhar informações sobre a forma como o JeffCrunch é utilizado. Anónimo significa que as informações não dizem nada sobre si. Quando esta opção está ativada, o JeffCrunch envia estas informações uma vez por dia e no dia em que atualizar: que se trata do JeffCrunch, a sua versão, o seu sistema operativo e idioma, quantas vezes foi iniciado, com que frequência cada comando é utilizado e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo deles, nem o que escreve. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: se a alterar aqui, fica alterada em todas elas. O JeffCrunch pergunta sobre isto na primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, ao lado da caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, mostra **nenhum**.
- Para que as estatísticas que já temos sejam eliminadas, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Ao desativá-las, o número é apagado do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira execução.

## Relacionados

- [Bem-vindo ao JeffCrunch](start)
