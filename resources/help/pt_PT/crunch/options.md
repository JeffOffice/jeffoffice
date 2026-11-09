# Alterar as definições do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais -->

As suas definições estão na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde segue a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir o restante de um texto a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto e apenas um texto dessa coluna começa por elas, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Só analisa o bloco de células ao lado da célula onde está a escrever, e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar da margem da seleção. Desative esta opção se move células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero em células com valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativada, uma célula com 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha fica guardada com a folha.
- **Introduzir um ponto decimal em números inteiros escritos** chama-se casas decimais automáticas. Defina **Casas** como 2 e, ao escrever 1234, é introduzido 12,34. Defina -2 e, ao escrever 5, é introduzido 500. Um número escrito com vírgula decimal é introduzido tal como foi escrito.

- **Permitir edição diretamente nas células** está ativada por predefinição. Quando a desativa, uma entrada escrita ou F2 vai para a barra de fórmulas, e a célula só mostra o resultado quando termina a entrada.

## Opções de guardar

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações ainda não guardadas, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, criada em segundo plano para que possa continuar a escrever. É eliminada quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar trabalho não guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações do JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar a ortografia ao escrever** para eliminar as linhas vermelhas onduladas; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação ortográfica, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas passam a aspas curvas, se os travessões, as frações e os ordinais são alterados, e se um endereço Web se transforma numa hiperligação. Veja [Ortografia](notes-links) e [Escrever nas células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã, não guardada no livro.
- **Zoom de um novo livro** indica a dimensão com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de todas as colunas desta folha que não tenham largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Opções de privacidade

- **Retirar o meu nome e a empresa das propriedades de um livro sempre que for guardado** limpa o autor, quem o guardou por último, a empresa e o gestor das propriedades do livro quando guarda. As propriedades são os dados que um ficheiro guarda sobre si próprio.

## Estatísticas de utilização

**Enviar estatísticas de utilização anónimas** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não revelam quem é. Quando esta opção está ativada, o JeffCrunch envia estes dados uma vez por dia, e no dia em que atualiza: que se trata do JeffCrunch, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência é usado cada comando e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar inesperadamente, também envia um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo, nem nada que tenha escrito. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações do JeffOffice: se a alterar aqui, altera-se em todas elas. O JeffCrunch pergunta sobre isto na primeira vez que é iniciado, na janela **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, junto à caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Indica o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório criado para a sua cópia do JeffOffice, igual em todas as aplicações do JeffOffice. Não diz nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido criado nenhum número, aparece **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos saber quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Assim, nada é contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira utilização.

## Consulte também

- [Bem-vindo ao JeffCrunch](start)
