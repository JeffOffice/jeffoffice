# Alterar as definições do JeffCrunch
<!-- keywords: recuperação, recuperar trabalho não guardado, recuperação automática, verificação, edição, preenchimento automático, alça de preenchimento, arrastar e largar, casas decimais, opções, definições, preferências, estatísticas de utilização, número de instalação, privacidade, aviso de privacidade, JP_NO_TELEMETRY, eliminar estatísticas, atualizações, acerca de, editar na célula, cor das linhas de grelha, zoom predefinido, largura padrão, privacidade, informações pessoais -->

As suas definições encontram-se na janela **Opções do JeffCrunch**. Clique em **Ficheiro > Opções** para a abrir. Clique em **OK** para manter as alterações ou em **Cancelar** para as descartar.

## Edição

A página **Edição** altera a forma como a escrita nas células funciona.

- **Depois de premir Enter, mover a seleção** indica para onde segue a seleção quando termina uma entrada: para baixo, para a direita, para cima, para a esquerda ou para lado nenhum.
- **Sugerir o texto a completar a partir das células acima e abaixo** chama-se Preenchimento Automático. Quando escreve as primeiras letras de um texto e apenas um texto dessa coluna começa por essas letras, o JeffCrunch mostra o resto, selecionado. Prima **Enter** para o manter. Continue a escrever para o ignorar, ou prima **Backspace** para o retirar. Esta função analisa apenas o bloco de células ao lado daquela em que está a escrever e apenas textos, não números.
- **Utilizar a alça de preenchimento e arrastar células para as mover ou copiar** ativa o pequeno quadrado no canto da seleção e o arrastar do limite da seleção. Desative-a se mover células por engano. Copiar e Colar continuam a funcionar.
- **Mostrar um zero em células com valor zero (esta folha)** ativa ou desativa os zeros na folha que está a ver. Quando está desativado, uma célula que contém 0 parece vazia, mas continua a conter o 0 e é somada como 0. A escolha é guardada com a folha.
- **Colocar um separador decimal em números inteiros introduzidos** chama-se casas decimais automáticas. Defina **Casas** como 2, e ao escrever 1234 obtém 12,34. Defina-as como -2, e ao escrever 5 obtém 500. Um número escrito com separador decimal é introduzido tal como escrito.

- **Permitir a edição diretamente nas células** está ativado por predefinição. Quando o desativa, uma entrada escrita ou F2 vai para a barra de fórmulas, e a célula só mostra o resultado quando termina a entrada.

## Opções de gravação

**Guardar informações de recuperação automática a cada** define a frequência com que o JeffCrunch guarda uma cópia de um livro com alterações que ainda não guardou, de 1 a 120 minutos. A cópia é um ficheiro .xlsx numa pasta que o JeffCrunch mantém para este fim, feita em segundo plano para que possa continuar a escrever. É eliminada quando guarda ou fecha o livro. Se o JeffCrunch fechar inesperadamente, oferece as cópias na próxima vez que for iniciado. Também pode clicar em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado**. Um livro protegido com palavra-passe não é copiado, porque a cópia não ficaria protegida.

## Opções de verificação

A página **Verificação** é partilhada pelas aplicações JeffOffice. Decide se a ortografia é verificada à medida que escreve numa célula (desative **Verificar a ortografia ao escrever** para eliminar as linhas vermelhas onduladas em todas as janelas abertas e nas outras aplicações; **Ortografia** no separador Rever continua a funcionar), se as palavras em maiúsculas e as palavras com números são ignoradas na verificação ortográfica, se a Correção Automática corrige palavras à medida que escreve, se as aspas retas passam a aspas curvas, se travessões, frações e ordinais são alterados e se um endereço Web se torna uma hiperligação. Consulte [Ortografia](notes-links) e [Escrever nas células](edit-cells).

## Opções de visualização

- **Cor das linhas de grelha** é a cor das linhas entre células no ecrã. É uma escolha apenas para o seu ecrã e não é guardada no livro.
- **Zoom de uma nova pasta de trabalho** define o tamanho com que as folhas de um novo livro são apresentadas, de 10% a 400%.
- **Largura padrão da coluna (esta folha)** é a largura de todas as colunas desta folha que não tenham largura própria. É o mesmo que **Largura Predefinida** no separador Base.

## Opções de privacidade

- **Retirar o meu nome e a empresa das propriedades de um livro sempre que for guardado** limpa o autor, quem o guardou por último, a empresa e o gestor das propriedades do livro quando guarda. As propriedades são os dados que um ficheiro guarda sobre si próprio.

## Estatísticas de utilização

**Enviar estatísticas de utilização anónimas** permite partilhar dados sobre a forma como o JeffCrunch é utilizado. Anónimo significa que os dados não revelam quem é. Quando esta opção está ativa, o JeffCrunch envia estes dados uma vez por dia e no dia em que atualizar: que se trata do JeffCrunch, a sua versão, o sistema operativo e o idioma, quantas vezes foi iniciado, com que frequência cada comando é utilizado e que tipo de ficheiro abriu ou guardou. Se o JeffCrunch fechar inesperadamente, envia também um relatório de falha na próxima vez que for iniciado. O JeffCrunch nunca envia os seus ficheiros, os respetivos nomes ou qualquer conteúdo, nem aquilo que escreveu. Se esta opção estiver desativada, também não é enviado nenhum relatório de falha.

A escolha aplica-se a todas as aplicações JeffOffice: se a alterar aqui, altera-se em todas elas. O JeffCrunch pergunta sobre esta opção da primeira vez que é iniciado, numa janela chamada **Ajude a melhorar o JeffCrunch**.

- **Aviso de privacidade**, junto à caixa de verificação, abre o nosso aviso de privacidade em jeffoffice.com/privacy. Nele indicamos o que é enviado, para onde vai e durante quanto tempo o guardamos. Guardamos as estatísticas durante 24 meses.
- **Número de instalação** é apresentado por baixo da caixa de verificação. É um número aleatório gerado para a sua cópia do JeffOffice, igual em todas as aplicações JeffOffice. Não revela nada sobre si. Clique em **Copiar** para o copiar. Se as estatísticas estiverem desativadas, ou se ainda não tiver sido gerado nenhum número, apresenta **nenhum**.
- Para eliminar as estatísticas que já temos, envie o seu número de instalação para info@jeffoffice.com **antes** de desativar as estatísticas. Desativá-las apaga o número do seu computador e, sem ele, não conseguimos identificar quais os registos que são seus.
- Para desativar as estatísticas sem abrir o JeffCrunch, defina a variável de ambiente `JP_NO_TELEMETRY` com qualquer valor antes de o iniciar. Nada é então contabilizado nem enviado, e o JeffCrunch não faz a pergunta da primeira utilização.

## Consulte também

- [Bem-vindo ao JeffCrunch](start)
